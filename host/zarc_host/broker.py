"""Single-owner, mutually authenticated lab guest gateway.

The downstream adapter must implement its own peer authentication for use
outside the lab. Do not expose the existing unauthenticated MCU fixture on a
vehicle or untrusted network. All socket operations occur on the C3 host path.
"""
import json
import socket
import ssl
import time
from dataclasses import dataclass
from .security import Policy

MAX_REQUEST = 512
MAX_RESPONSE = 512
MAX_RETAINED = 16

def identity(value):
    return type(value) is int and 1 <= value <= 0xFFFFFFFFFFFFFFFF

@dataclass(frozen=True)
class Command:
    operation: str
    capability: int
    desired: int | None = None
    validity: int | None = None
    boot: int | None = None
    session: int | None = None
    request_id: int | None = None

def parse_command(data):
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError("duplicate field")
            result[key] = value
        return result
    if not data or len(data) > MAX_REQUEST:
        raise ValueError("request size")
    obj = json.loads(data, object_pairs_hook=pairs)
    if not isinstance(obj, dict):
        raise ValueError("object required")
    op = obj.get("operation")
    if op=="update_status" and set(obj)=={"operation"}:
        return Command(op,0)
    domain = obj.get("capability")
    if type(domain) is not int or not 1 <= domain <= 0xFFFFFFFF:
        raise ValueError("capability")
    if op in ("snapshot","health") and set(obj) == {"operation", "capability"}:
        return Command(op, domain)
    if op == "request_status" and set(obj) == {"operation","capability","boot","session","request_id"}:
        if all(identity(obj[k]) for k in ("boot","session","request_id")):
            return Command(op,domain,boot=obj["boot"],session=obj["session"],request_id=obj["request_id"])
    if op == "request" and set(obj) == {"operation", "capability", "desired", "validity"}:
        desired, validity = obj["desired"], obj["validity"]
        if type(desired) is int and desired in (1, 2) and type(validity) is int and 1 <= validity <= 1000:
            return Command(op, domain, desired, validity)
    raise ValueError("unsupported command")

class Gateway:
    """One serialized owner of downstream adapter; no automatic write retry."""
    def __init__(self, policy: Policy, backend):
        self.policy = policy
        self.backend = backend
        self._epoch = None
        self._owners = {}
        # A restarted broker has no trustworthy observation state. A fresh
        # successful snapshot is the barrier before it may issue writes.
        self._snapshot_ready = False

    def _clear_state(self):
        self._owners.clear()
        self._epoch = None
        self._snapshot_ready = False

    def _observe_backend_epoch(self, *, snapshot=False):
        boot = getattr(self.backend, "boot", None)
        session = getattr(self.backend, "session", None)
        epoch = (boot, session) if identity(boot) and identity(session) else None
        if epoch is not None and epoch != self._epoch:
            self._owners.clear()
            self._epoch = epoch
            self._snapshot_ready = False
        if snapshot:
            self._snapshot_ready = epoch is not None

    def dispatch(self, principal, raw):
        try:
            cmd = parse_command(raw)
        except (ValueError, TypeError, UnicodeError, RecursionError):
            return {"status": "INVALID"}
        if not self.policy.authorize(principal, cmd.operation, cmd.capability):
            return {"status": "DENIED"}
        try:
            if cmd.operation == "request_status":
                key=(cmd.boot,cmd.session,cmd.request_id)
                # Do not connect/replay an old request or disclose its owner.
                if self._owners.get(key)!=(principal.name,cmd.capability):
                    return {"status":"UNKNOWN_OUTCOME"}
                return self.backend.request_status(cmd.request_id,boot=cmd.boot,session=cmd.session)
            if cmd.operation == "update_status":
                result=self.backend.update_status()
                self._observe_backend_epoch()
                return result
            if cmd.operation == "health":
                result=self.backend.health(cmd.capability)
                self._observe_backend_epoch()
                return result
            if cmd.operation == "snapshot":
                result=self.backend.snapshot(cmd.capability)
                self._observe_backend_epoch(snapshot=result.get("status")=="COMPLETED")
                return result
            if not self._snapshot_ready:
                return {"status":"UNAVAILABLE"}
            result=self.backend.request(cmd.capability,cmd.desired,cmd.validity)
            if result.get("status")=="ACCEPTED":
                boot,session,rid=(result.get(k) for k in ("boot","session","request_id"))
                if not all(identity(x) for x in (boot,session,rid)):
                    # Never expose acceptance without a queryable immutable
                    # identity for the downstream retained outcome.
                    self.backend.invalidate()
                    self._clear_state()
                    return {"status":"UNKNOWN_OUTCOME"}
                epoch=(boot,session)
                if epoch!=self._epoch:
                    self._owners.clear()
                    self._epoch=epoch
                    self._snapshot_ready=False
                    return {"status":"UNKNOWN_OUTCOME"}
                key=(boot,session,rid)
                owner=(principal.name,cmd.capability)
                if key in self._owners and self._owners[key]!=owner:
                    self.backend.invalidate()
                    self._clear_state()
                    return {"status":"UNKNOWN_OUTCOME"}
                if key not in self._owners and len(self._owners)>=MAX_RETAINED:
                    # Unexpected backend violation: an admission may have
                    # occurred, but cannot safely expose its retained result.
                    self.backend.invalidate()
                    self._clear_state()
                    return {"status":"UNKNOWN_OUTCOME"}
                self._owners[key]=owner
            return result
        except (OSError, TimeoutError, ValueError):
            self.backend.invalidate()
            self._clear_state()
            # Never assert an admitted request was not executed after link loss.
            return {"status": "UNKNOWN_OUTCOME" if cmd.operation in ("request","request_status") else "UNAVAILABLE"}

    def serve_connection(self, raw: socket.socket, context: ssl.SSLContext):
        # One bounded request per TLS session, then close; no guest-selected identity.
        raw.settimeout(2)
        with context.wrap_socket(raw, server_side=True) as peer:
            principal = self.policy.identify(peer)
            data = bytearray()
            deadline = time.monotonic() + 2
            while len(data) <= MAX_REQUEST:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError("request deadline")
                peer.settimeout(remaining)
                chunk = peer.recv(1)
                if not chunk:
                    return
                if chunk == b"\n":
                    break
                data += chunk
            peer.settimeout(2)
            result = self.dispatch(principal, bytes(data))
            encoded = json.dumps(result, separators=(",", ":"), allow_nan=False).encode()
            if len(encoded) > MAX_RESPONSE:
                encoded = b'{"status":"UNAVAILABLE"}'
            peer.sendall(encoded + b"\n")

    def serve(self, listener, context, stop, *, accept_timeout=0.2):
        """Trusted loopback listener supplied by deployment, one guest at a time."""
        if listener.getsockname()[0] not in ("127.0.0.1", "::1"):
            raise ValueError("lab gateway is loopback only")
        listener.settimeout(accept_timeout)
        while not stop():
            try:
                raw, _ = listener.accept()
            except socket.timeout:
                continue
            with raw:
                try:
                    self.serve_connection(raw, context)
                except (OSError, PermissionError, ValueError):
                    pass
