"""Host-side mutual TLS and least-privilege capability policy.

TLS authenticates the guest to the trusted broker. It does not authenticate the
MCU link; that separate trust boundary must be qualified by the integrator.
"""
from dataclasses import dataclass
from hashlib import sha256
import ssl
import time

@dataclass(frozen=True)
class Principal:
    name: str
    read: frozenset[int]
    write: frozenset[int]
    rate: int = 8

    def __post_init__(self):
        if not self.name or not 1 <= self.rate <= 100:
            raise ValueError("invalid principal policy")
        if any(type(x) is not int or not 1 <= x <= 0xFFFFFFFF
               for x in self.read | self.write):
            raise ValueError("invalid capability")

def server_context(certificate, private_key, trust_store):
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.verify_mode = ssl.CERT_REQUIRED
    ctx.load_cert_chain(certificate, private_key)
    ctx.load_verify_locations(cafile=trust_store)
    ctx.options |= ssl.OP_NO_TICKET
    return ctx

def client_context(certificate, private_key, trust_store):
    ctx = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile=trust_store)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.load_cert_chain(certificate, private_key)
    return ctx

class Policy:
    """Allowlist binds a verified DER certificate to immutable scopes.
    Rate limit is shared across reconnects for the same named principal.
    Construct only from approved local configuration.
    """
    def __init__(self, certificates, clock=time.monotonic):
        if not certificates or len(certificates) > 64:
            raise ValueError("certificate capacity")
        self._entries = {}
        self._usage = {}
        self._clock = clock
        names = {}
        for digest, principal in certificates.items():
            if len(digest) != 64 or any(c not in "0123456789abcdef" for c in digest):
                raise ValueError("invalid SHA256 fingerprint")
            if principal.name in names and names[principal.name] != principal:
                raise ValueError("inconsistent rotated certificate policy")
            names[principal.name] = principal
            self._entries[digest] = principal

    def identify(self, peer):
        if not isinstance(peer, ssl.SSLSocket):
            raise PermissionError("TLS required")
        if peer.context.verify_mode != ssl.CERT_REQUIRED or peer.version() != "TLSv1.3":
            raise PermissionError("verified TLS 1.3 required")
        der = peer.getpeercert(binary_form=True)
        principal = self._entries.get(sha256(der).hexdigest()) if der else None
        if principal is None:
            raise PermissionError("unknown certificate")
        return principal

    def authorize(self, principal, operation, capability):
        if principal not in self._entries.values():
            return False
        scope = principal.read if operation == "snapshot" else (
            principal.write if operation == "request" else frozenset())
        if type(capability) is not int or capability not in scope:
            return False
        now = self._clock()
        window, used = self._usage.get(principal.name, (now, 0))
        if now < window:
            return False
        if now - window >= 1:
            window, used = now, 0
        if used >= principal.rate:
            return False
        self._usage[principal.name] = (window, used + 1)
        return True
