# ZARC — Zephyr Automotive Real-Time Control

ZARC is a Zephyr-first automotive controller and integration framework with a portable C17 core. The repository includes desktop, host-service, and native Zephyr simulator references; these do not claim physical target support or product qualification. Open `ZARC.code-workspace` in VS Code. CMake 3.21+ and a C17 compiler are required for the desktop quick start.

## Quick start

### Windows

```powershell
cmake --preset desktop-windows
cmake --build --preset desktop-windows
ctest --preset desktop-windows
```

### Linux

```bash
cmake --preset desktop-linux
cmake --build --preset desktop-linux
ctest --preset desktop-linux
```
Use the preset matching your host. Windows requires MSVC and CMake available in the shell; Linux requires Ninja and GCC or Clang. Keep separate build directories for each operating system.

The desktop reference uses a simulated indicator. Its controller and telemetry bridge do not operate physical vehicle hardware; the bridge is read-only. See [implementation status](docs/IMPLEMENTATION-STATUS.md) and the [authoritative requirement trace](docs/REQUIREMENT-TRACE.csv) for implemented software paths and open acceptance evidence.

### Show reference-controller telemetry in the sedan

Build `zarc_controller_lab_pc` using the desktop preset and start it with a nonzero boot ID, for example `./build/linux/zarc_controller_lab_pc 42`. It prints a local TCP port. In another terminal run `PYTHONPATH=host python tools/telemetry_bridge.py --controller-port PORT`, replacing `PORT` with that value. Open the [sedan reference page](https://susmithmrm.github.io/ZARC/modern-architecture.html) and connect to `http://127.0.0.1:8765/snapshot`. The display is read-only and identifies the desktop controller as simulated; loss of fresh valid samples changes the label to stale. This does not connect physical vehicle hardware.

See [implementation status](docs/IMPLEMENTATION-STATUS.md) and the [requirement trace](docs/REQUIREMENT-TRACE.csv) for implemented software and remaining evidence. The repository includes independent desktop controller/broker processes, session and request retention, health and diagnostics, persistent configuration and update journals, and native Zephyr simulator profiles. Physical target ports, product-specific allocations and qualification remain open. No real automotive hardware is operated by the reference application.

## Suggested modern vehicle architecture

[Rendered reference design on GitHub Pages](https://susmithmrm.github.io/ZARC/modern-architecture.html) · [Engineering topology and authority boundaries](docs/architecture/18-Modern-SDV-Reference-Architecture.md). This proposed topology is informative and does not change the controlled requirements baseline.

## Integrate with ZARC

[Platform capabilities and partner adoption](https://susmithmrm.github.io/ZARC/platform-adoption.html) · [Engineering adoption guide](docs/architecture/19-Platform-Capabilities-and-Partner-Adoption.md). Choose a deployment profile, target and role; follow candidate scope and evidence gates.

## Documentation

[Architecture index](docs/architecture/00-README.md) · [Diagram atlas](docs/architecture/diagrams/README.md) · [Project site](https://susmithmrm.github.io/ZARC/) · [Review findings](docs/REPOSITORY-AUDIT.md)

## License

ZARC is licensed under the [MIT License](LICENSE). Third-party dependencies retain their respective licenses.

The authoritative requirements register is [Z12](docs/architecture/12-Requirements-Verification.csv). Reference prototype, demonstration and BOM are linked from the architecture index.
