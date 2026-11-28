"""Read-only localhost bridge from the desktop lab controller to the sedan twin.

The bridge exposes observations only. It binds loopback and does not provide a
request/actuation path. Data is explicitly marked as simulated-controller data.
"""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import threading
import time

from zarc_host.lab_backend import LabBackend


class TelemetrySource:
    def __init__(self, backend, capability=1, poll_seconds=0.2, clock_ms=None):
        if type(capability) is not int or capability <= 0:
            raise ValueError("capability must be positive")
        if type(poll_seconds) not in (int, float) or poll_seconds <= 0:
            raise ValueError("poll interval must be positive")
        self.backend = backend
        self.capability = capability
        self.poll_seconds = poll_seconds
        self.clock_ms = clock_ms or (lambda: int(time.time() * 1000))
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread = None
        self._last_good = None
        self._state = {"schema": 1, "status": "UNAVAILABLE",
                       "provenance": "simulated_controller",
                       "captured_at_ms": self.clock_ms(),
                       "capability": capability}

    def start(self):
        if self._thread is not None:
            return
        self._thread = threading.Thread(target=self._run, name="zarc-telemetry",
                                        daemon=True)
        self._thread.start()

    def stop(self):
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=2)
        self.backend.invalidate()

    def snapshot(self):
        with self._lock:
            result = dict(self._state)
        if self._last_good is not None:
            result["last_good"] = dict(self._last_good)
        return result

    def poll_once(self):
        now = self.clock_ms()
        try:
            observation = self.backend.snapshot(self.capability)
            if observation.get("status") == "COMPLETED":
                if observation.get("quality") not in (0, 1, 2, 3):
                    raise ValueError("invalid observation quality")
                self._last_good = {
                    "value": observation["value"],
                    "quality": observation["quality"],
                    "generation": observation["generation"],
                    "boot": observation["boot"],
                    "captured_at_ms": now,
                }
                state = {"schema": 1, "status": "COMPLETED",
                         "provenance": "simulated_controller",
                         "captured_at_ms": now,
                         "capability": self.capability,
                         **self._last_good}
            else:
                state = {"schema": 1, "status": observation.get("status", "UNAVAILABLE"),
                         "provenance": "simulated_controller",
                         "captured_at_ms": now,
                         "capability": self.capability}
        except (OSError, ValueError, KeyError, TypeError):
            self.backend.invalidate()
            state = {"schema": 1, "status": "UNAVAILABLE",
                     "provenance": "simulated_controller",
                     "captured_at_ms": now,
                     "capability": self.capability}
        with self._lock:
            self._state = state
        return self.snapshot()

    def _run(self):
        while not self._stop.is_set():
            self.poll_once()
            self._stop.wait(self.poll_seconds)


def make_handler(source, allowed_origins):
    class Handler(BaseHTTPRequestHandler):
        def _cors(self):
            origin = self.headers.get("Origin")
            if origin in allowed_origins:
                self.send_header("Access-Control-Allow-Origin", origin)
                self.send_header("Vary", "Origin")

        def do_OPTIONS(self):
            origin = self.headers.get("Origin")
            if origin not in allowed_origins:
                self.send_error(403)
                return
            self.send_response(204)
            self._cors()
            self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "Content-Type")
            self.end_headers()

        def do_GET(self):
            if self.path != "/snapshot":
                self.send_error(404)
                return
            origin = self.headers.get("Origin")
            if origin and origin not in allowed_origins:
                self.send_error(403)
                return
            body = json.dumps(source.snapshot(), separators=(",", ":")).encode()
            self.send_response(200)
            self._cors()
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, _format, *_args):
            pass
    return Handler


def serve(controller_port, http_port=8765, poll_ms=200, allowed_origins=None):
    source = TelemetrySource(LabBackend(controller_port), poll_seconds=poll_ms / 1000)
    origins = set(allowed_origins or ("https://susmithmrm.github.io",))
    source.start()
    server = ThreadingHTTPServer(("127.0.0.1", http_port), make_handler(source, origins))
    try:
        print(f"Read-only ZARC telemetry at http://127.0.0.1:{server.server_port}/snapshot", flush=True)
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.shutdown()
        server.server_close()
        source.stop()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--controller-port", type=int, required=True,
                        help="port printed by zarc_controller_lab_pc")
    parser.add_argument("--http-port", type=int, default=8765)
    parser.add_argument("--poll-ms", type=int, default=200)
    parser.add_argument("--allow-origin", action="append", default=[],
                        help="additional exact page origin allowed to read snapshots")
    args = parser.parse_args()
    if not 1 <= args.controller_port <= 65535 or not 0 <= args.http_port <= 65535:
        parser.error("ports must be in 1..65535 (HTTP also accepts 0 for an ephemeral port)")
    if not 20 <= args.poll_ms <= 5000:
        parser.error("--poll-ms must be between 20 and 5000")
    serve(args.controller_port, args.http_port, args.poll_ms, args.allow_origin)


if __name__ == "__main__":
    main()
