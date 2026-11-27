# Native provider conformance candidates

The [provider Twister application](../platform/zephyr/provider-tests/testcase.yaml) selects real Zephyr UART async, CAN and NVS subsystems under native_sim. It exercises async UART buffer ownership with an emulator, CAN loopback, and a two-slot configuration record through an actual NVS mount/remount. The simulator test clears only its dedicated storage partition; it must not run on a production partition.

The IPC fixture implements a small explicit simulated backend behind the native IPC service API, verifies copy ownership and rejects work after unbound. The watchdog spy device records native driver API calls so a failing health decision cannot feed it. The MCUboot variant compiles the optional boot adapter against the image manager; it does not run a bootloader. The provider test suite is an additional native API conformance layer, not physical bus, reset, flash or shared-memory evidence.

Trace candidates: CON-010/011/017/018, NET-002, RT-016/020, SVC-010, ASR-007. A successful Twister run is required before calling the fixture verified; selected hardware and independent HIL remain open.
