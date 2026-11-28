# ZARC — Zephyr Automotive Real-Time Control

Portable C17 first slice using a simulated indicator. Open ZARC.code-workspace in VS Code. CMake 3.21+ and a C17 compiler are required.

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

 The tests cover admission, feedback confirmation, timeout, duplicate handling and independent instances. The starter retains only one request per instance, refuses older IDs and assumes a single simulated principal/session. It is a prototype, not a full contract implementation.

### Show reference-controller telemetry in the sedan

Build `zarc_controller_lab_pc` using the desktop preset and start it with a nonzero boot ID, for example `./build/linux/zarc_controller_lab_pc 42`. It prints a local TCP port. In another terminal run `PYTHONPATH=host python tools/telemetry_bridge.py --controller-port PORT`, replacing `PORT` with that value. Open the [sedan reference page](https://susmithmrm.github.io/ZARC/modern-architecture.html) and connect to `http://127.0.0.1:8765/snapshot`. The display is read-only and identifies the desktop controller as simulated; loss of fresh valid samples changes the label to stale. This does not connect physical vehicle hardware.

See [implementation status](docs/IMPLEMENTATION-STATUS.md) for the current scope and [implementation realization](docs/architecture/realization/16-Implementation-Realization.md) for planned work. Future increments include independent broker processes, service registry, session contracts, diagnostics, persistence, target adapters and target evidence. No real automotive hardware is operated by this starter.

## Suggested modern vehicle architecture

[Rendered reference design on GitHub Pages](https://susmithmrm.github.io/ZARC/modern-architecture.html) · [Engineering topology and authority boundaries](docs/architecture/18-Modern-SDV-Reference-Architecture.md). This proposed topology is informative and does not change the controlled requirements baseline.

## Integrate with ZARC

[Platform capabilities and partner adoption](https://susmithmrm.github.io/ZARC/platform-adoption.html) · [Engineering adoption guide](docs/architecture/19-Platform-Capabilities-and-Partner-Adoption.md). Choose a deployment profile, target and role; follow candidate scope and evidence gates.

## Documentation

[Architecture index](docs/architecture/00-README.md) · [Diagram atlas](docs/architecture/diagrams/README.md) · [Project site](https://susmithmrm.github.io/ZARC/) · [Review findings](docs/REPOSITORY-AUDIT.md)

## License

ZARC is licensed under the [MIT License](LICENSE). Third-party dependencies retain their respective licenses.

The authoritative requirements register is [Z12](docs/architecture/12-Requirements-Verification.csv). Reference prototype, demonstration and BOM are linked from the architecture index.
