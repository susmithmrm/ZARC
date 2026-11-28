import json
import subprocess
import sys
import threading
import unittest
from http.server import ThreadingHTTPServer
from urllib.error import HTTPError
from urllib.request import Request, urlopen
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "host"))
sys.path.insert(0, str(ROOT / "tools"))
from telemetry_bridge import TelemetrySource, make_handler
from zarc_host.lab_backend import LabBackend


class Backend:
    def __init__(self, result):
        self.result = result
        self.invalidations = 0

    def snapshot(self, _capability):
        if isinstance(self.result, Exception):
            raise self.result
        return self.result

    def invalidate(self):
        self.invalidations += 1


class TelemetryBridgeTests(unittest.TestCase):
    def test_valid_observation_retains_provenance_and_source_identity(self):
        backend = Backend(dict(status="COMPLETED", value=1, quality=1,
                               generation=9, boot=55))
        source = TelemetrySource(backend, clock_ms=lambda: 1234)
        state = source.poll_once()
        self.assertEqual(state["status"], "COMPLETED")
        self.assertEqual(state["provenance"], "simulated_controller")
        self.assertEqual((state["value"], state["quality"], state["generation"],
                          state["boot"], state["captured_at_ms"]), (1, 1, 9, 55, 1234))

    def test_invalid_peer_data_fails_closed_and_keeps_last_sample(self):
        backend = Backend(dict(status="COMPLETED", value=1, quality=9,
                               generation=9, boot=55))
        source = TelemetrySource(backend, clock_ms=lambda: 100)
        state = source.poll_once()
        self.assertEqual(state["status"], "UNAVAILABLE")
        self.assertNotIn("last_good", state)
        self.assertEqual(backend.invalidations, 1)

    def test_http_is_read_only_and_origin_scoped(self):
        source = TelemetrySource(Backend({"status": "UNAVAILABLE"}))
        server = ThreadingHTTPServer(("127.0.0.1", 0),
                                     make_handler(source, {"https://susmithmrm.github.io"}))
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            url = f"http://127.0.0.1:{server.server_port}/snapshot"
            req = Request(url, headers={"Origin": "https://susmithmrm.github.io"})
            with urlopen(req, timeout=2) as response:
                payload = json.loads(response.read())
                self.assertEqual(response.headers["Access-Control-Allow-Origin"],
                                 "https://susmithmrm.github.io")
                self.assertEqual(payload["status"], "UNAVAILABLE")
                self.assertEqual(response.headers["Cache-Control"], "no-store")
            bad_origin = Request(url, headers={"Origin": "https://untrusted.example"})
            with self.assertRaises(HTTPError) as error:
                urlopen(bad_origin, timeout=2)
            self.assertEqual(error.exception.code, 403)
            with self.assertRaises(HTTPError) as missing:
                urlopen(url.replace("/snapshot", "/control"), timeout=2)
            self.assertEqual(missing.exception.code, 404)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=2)


def test_controller_integration(binary):
    process = subprocess.Popen([binary, "71"], stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, text=True)
    source = None
    try:
        line = process.stdout.readline().strip()
        if not line.isdecimal():
            raise AssertionError("reference controller did not publish its port")
        source = TelemetrySource(LabBackend(int(line)))
        state = source.poll_once()
        if state["status"] != "COMPLETED" or state["provenance"] != "simulated_controller":
            raise AssertionError(f"unexpected controller observation: {state}")
        if state["value"] not in (0, 1) or state["quality"] != 1 or state["boot"] != 71:
            raise AssertionError(f"unexpected controller sample: {state}")
    finally:
        if source is not None:
            source.stop()
        process.terminate()
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=2)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        test_controller_integration(sys.argv[1])
        sys.argv = sys.argv[:1]
    unittest.main()
