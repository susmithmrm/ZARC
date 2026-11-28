"""Deterministic scenario replay across independent desktop processes."""
import subprocess
import sys
import time

def scenario():
    controller = subprocess.Popen([sys.argv[1], "101"], stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE, text=True)
    try:
        port = controller.stdout.readline().strip()
        if not port.isdecimal():
            raise RuntimeError("controller did not publish a port")

        def broker(action):
            return subprocess.run([sys.argv[2], port, action], capture_output=True,
                                  text=True, timeout=4, check=False)

        first = broker("snapshot")
        assert first.returncode == 0 and first.stdout.strip() == "SNAPSHOT 1 1", first
        accepted = broker("on")
        assert accepted.returncode == 0 and accepted.stdout.strip() == "ACCEPTED", accepted
        # Broker has exited. Controller must finish using its own virtual device.
        deadline = time.monotonic() + 5
        while True:
            second = broker("snapshot")
            if second.returncode == 0:
                assert second.stdout.strip() == "SNAPSHOT 2 1", second
                break
            assert second.returncode == 3 and time.monotonic() < deadline, second
            time.sleep(0.05)
        assert controller.poll() is None
        # Port, session, and wall-clock timing are intentionally excluded.
        return (first.stdout.strip(), accepted.stdout.strip(), second.stdout.strip())
    finally:
        controller.terminate()
        try:
            controller.communicate(timeout=3)
        except subprocess.TimeoutExpired:
            controller.kill()
            controller.communicate()


first_run = scenario()
second_run = scenario()
assert first_run == second_run, (first_run, second_run)
print("Repeated virtual ECU scenario produced identical observations")
