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


# Deterministic, simulation-only reference bench signals. This catalogue is
# separate from TelemetrySource and never represents controller feedback.
SIMULATED_CHANNEL_SPECS = (
    ("BENCH-001", "Bench supply voltage", "power", "input", 12.4, "V", 0.0, 16.0),
    ("BENCH-002", "Bench branch current", "power", "input", 0.55, "A", 0.0, 5.0),
    ("BENCH-003", "Controller board temperature", "environment", "input", 34.0, "degC", -40.0, 125.0),
    ("BENCH-004", "Ambient temperature", "environment", "input", 22.5, "degC", -40.0, 85.0),
    ("BENCH-005", "Cabin humidity", "environment", "input", 47.0, "%", 0.0, 100.0),
    ("BENCH-006", "Ambient light", "environment", "input", 315.0, "lx", 0.0, 100000.0),
    ("BENCH-007", "Rain sensor level", "environment", "input", 4.0, "%", 0.0, 100.0),
    ("BENCH-008", "Front left time-of-flight distance", "distance", "input", 648.0, "mm", 0.0, 4000.0),
    ("BENCH-009", "Front right time-of-flight distance", "distance", "input", 652.0, "mm", 0.0, 4000.0),
    ("BENCH-010", "Rear left time-of-flight distance", "distance", "input", 1208.0, "mm", 0.0, 4000.0),
    ("BENCH-011", "Rear right time-of-flight distance", "distance", "input", 1192.0, "mm", 0.0, 4000.0),
    ("BENCH-012", "Front left ultrasonic distance", "distance", "input", 711.0, "mm", 0.0, 5000.0),
    ("BENCH-013", "Front right ultrasonic distance", "distance", "input", 704.0, "mm", 0.0, 5000.0),
    ("BENCH-014", "Rear left ultrasonic distance", "distance", "input", 1360.0, "mm", 0.0, 5000.0),
    ("BENCH-015", "Rear right ultrasonic distance", "distance", "input", 1373.0, "mm", 0.0, 5000.0),
    ("BENCH-016", "Front left wheel Hall pulses", "motion", "input", 1200.0, "count", 0.0, 65535.0),
    ("BENCH-017", "Front right wheel Hall pulses", "motion", "input", 1182.0, "count", 0.0, 65535.0),
    ("BENCH-018", "Rear left wheel Hall pulses", "motion", "input", 1213.0, "count", 0.0, 65535.0),
    ("BENCH-019", "Rear right wheel Hall pulses", "motion", "input", 1207.0, "count", 0.0, 65535.0),
    ("BENCH-020", "Steering encoder angle", "motion", "input", 2.5, "deg", -720.0, 720.0),
    ("BENCH-021", "Driver door switch", "body", "input", 0.0, "state", 0.0, 1.0),
    ("BENCH-022", "Passenger door switch", "body", "input", 0.0, "state", 0.0, 1.0),
    ("BENCH-023", "Rear left door switch", "body", "input", 1.0, "state", 0.0, 1.0),
    ("BENCH-024", "Rear right door switch", "body", "input", 0.0, "state", 0.0, 1.0),
    ("BENCH-025", "Trunk switch", "body", "input", 0.0, "state", 0.0, 1.0),
    ("BENCH-026", "Hood switch", "body", "input", 0.0, "state", 0.0, 1.0),
    ("BENCH-027", "Front left window position", "body", "feedback", 100.0, "%", 0.0, 100.0),
    ("BENCH-028", "Front right window position", "body", "feedback", 100.0, "%", 0.0, 100.0),
    ("BENCH-029", "Rear left window position", "body", "feedback", 40.0, "%", 0.0, 100.0),
    ("BENCH-030", "Rear right window position", "body", "feedback", 40.0, "%", 0.0, 100.0),
    ("BENCH-031", "Seat load sensor", "body", "input", 12.0, "kg", 0.0, 200.0),
    ("BENCH-032", "Cooling fan tachometer", "actuator", "feedback", 840.0, "rpm", 0.0, 8000.0),
    ("BENCH-033", "Wiper position feedback", "actuator", "feedback", 0.0, "deg", -90.0, 90.0),
    ("BENCH-034", "Front left lamp feedback", "actuator", "feedback", 1.0, "state", 0.0, 1.0),
    ("BENCH-035", "Front right lamp feedback", "actuator", "feedback", 1.0, "state", 0.0, 1.0),
    ("BENCH-036", "Rear left lamp feedback", "actuator", "feedback", 1.0, "state", 0.0, 1.0),
    ("BENCH-037", "Rear right lamp feedback", "actuator", "feedback", 1.0, "state", 0.0, 1.0),
    ("BENCH-038", "Cabin lamp feedback", "actuator", "feedback", 0.0, "state", 0.0, 1.0),
    ("BENCH-039", "Left indicator feedback", "actuator", "feedback", 0.0, "state", 0.0, 1.0),
    ("BENCH-040", "Right indicator feedback", "actuator", "feedback", 0.0, "state", 0.0, 1.0),
    ("BENCH-041", "Cooling fan output duty", "actuator", "feedback", 28.0, "%", 0.0, 100.0),
    ("BENCH-042", "Wiper servo position", "actuator", "feedback", 0.0, "deg", -90.0, 90.0),
    ("BENCH-043", "Left window motor state", "actuator", "feedback", 0.0, "state", 0.0, 1.0),
    ("BENCH-044", "Right window motor state", "actuator", "feedback", 0.0, "state", 0.0, 1.0),
    ("BENCH-045", "Central lock feedback", "body", "feedback", 1.0, "state", 0.0, 1.0),
    ("BENCH-046", "CAN link state", "network", "status", 1.0, "state", 0.0, 1.0),
    ("BENCH-047", "UART link state", "network", "status", 1.0, "state", 0.0, 1.0),
    ("BENCH-048", "IPC queue depth", "network", "status", 0.0, "count", 0.0, 64.0),
    ("BENCH-049", "Controller health state", "controller", "status", 1.0, "state", 0.0, 1.0),
    ("BENCH-050", "Diagnostic fault count", "controller", "status", 0.0, "count", 0.0, 255.0),
)


def validate_simulated_channels(channels):
    """Validate the fixed 50-channel fixture, including provenance and ranges."""
    if not isinstance(channels, (tuple, list)) or len(channels) != 50:
        raise ValueError("simulated reference catalogue must contain exactly 50 channels")
    seen = set()
    required = {"id", "name", "group", "direction", "value", "unit", "quality",
                "source", "sample_tick_ms", "provenance", "value_min", "value_max"}
    for channel in channels:
        if not isinstance(channel, dict) or set(channel) != required:
            raise ValueError("simulated channel schema mismatch")
        ident = channel["id"]
        if not isinstance(ident, str) or not ident or ident in seen:
            raise ValueError("simulated channel ID missing or duplicated")
        seen.add(ident)
        if any(not isinstance(channel[k], str) or not channel[k].strip()
               for k in ("name", "group", "direction", "unit", "source")):
            raise ValueError("simulated channel metadata missing")
        if channel["direction"] not in ("input", "feedback", "status"):
            raise ValueError("invalid simulated channel direction")
        if (type(channel["value"]) not in (int, float) or
                type(channel["value_min"]) not in (int, float) or
                type(channel["value_max"]) not in (int, float) or
                not channel["value_min"] <= channel["value"] <= channel["value_max"]):
            raise ValueError("simulated channel value outside declared range")
        if (channel["quality"] != "SIMULATED" or
                channel["provenance"] != "simulated_reference_bench" or
                type(channel["sample_tick_ms"]) is not int or
                channel["sample_tick_ms"] != 0):
            raise ValueError("simulated channel provenance or time invalid")
    return True


def simulated_channel_catalogue():
    channels = [{"id": ident, "name": name, "group": group, "direction": direction,
                 "value": value, "unit": unit, "quality": "SIMULATED",
                 "source": "deterministic_reference_scenario", "sample_tick_ms": 0,
                 "provenance": "simulated_reference_bench", "value_min": low,
                 "value_max": high}
                for ident, name, group, direction, value, unit, low, high
                in SIMULATED_CHANNEL_SPECS]
    validate_simulated_channels(channels)
    return {"schema": 1, "catalogue_id": "reference-bench-50-v1",
            "status": "SIMULATED", "provenance": "simulated_reference_bench",
            "sample_tick_ms": 0, "channels": channels}


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
            if self.path == "/channels":
                body = json.dumps(simulated_channel_catalogue(), separators=(",", ":"),
                                  allow_nan=False).encode()
            elif self.path == "/snapshot":
                body = json.dumps(source.snapshot(), separators=(",", ":"),
                                  allow_nan=False).encode()
            else:
                self.send_error(404)
                return
            origin = self.headers.get("Origin")
            if origin and origin not in allowed_origins:
                self.send_error(403)
                return
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
