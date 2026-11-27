# Native Zephyr laboratory fixture

This application compiles the shared portable libraries into a Zephyr ztest fixture. It exercises native bounded message queues, a separate device worker, monotonic uptime and a single controller owner. The worker emits independent simulated feedback after the controller link is disconnected. No real hardware is operated.

Candidate development pin: Zephyr v4.2.0, subject to integrator review. From an initialized Zephyr workspace with the repository checked out:

```sh
west twister -T /path/to/ZARC/platform/zephyr -p native_sim/native/64 --inline-logs
```

Or run the native simulator directly:

```sh
west build -b native_sim/native/64 /path/to/ZARC/platform/zephyr
west build -t run
```

Requires a compatible Zephyr toolchain and host simulator dependencies. The current authoring environment cannot install the Zephyr workspace or run this test. Treat the pin and build as unverified candidates until a recorded Twister run succeeds.

No UART/CAN/IPC/Ethernet driver mapping, physical GPIO, PM, watchdog, flash, bootloader, real-time measurements, board admission or target safety evidence is supplied by this fixture. Actual selected-target configuration and approved hardware policy remain necessary.
