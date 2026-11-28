# ZARC — Zephyr Automotive Real-Time Control

**Open, modular, observable and continuously evolvable real-time software for intelligent vehicles and embedded systems.**

ZARC aims to become the preferred open-source real-time platform for automotive OEMs, Tier 1 and Tier 2 suppliers, semiconductor manufacturers and embedded developers by 2030–2035, while remaining compatible with today's distributed electronic architectures.

| Deployment | Intended profile |
|---|---|
| Distributed CAN/CAN FD/LIN ECU | Edge (E) |
| Hybrid/domain and central compute companion | Companion (C) |
| Zonal I/O and power supervision | Zone (Z) |
| Heterogeneous SoC real-time domain | Island (I) |

## Architecture principles

- Native Zephyr first, portable C17 domain logic, independently versioned components and static composition by default.
- Deployment-unit replacement and update policy; compatible state migration and approved recovery.
- Authorized system visibility: topology, health, diagnostics, timing, resources, versions and provenance.
- Local deterministic authority independent of host, cloud and advisory AI; management is bounded.
- Open interfaces, multi-vendor portability, measurable lifecycle economics, one requirements baseline.

**Status:** engineering reference only. The proposed 267-entry [Z12](docs/architecture/12-Requirements-Verification.csv) describes the intended design, not completed capabilities. See [implementation evidence](docs/IMPLEMENTATION-STATUS.md) and the [derived trace](docs/REQUIREMENT-TRACE.csv). Physical ports, independent assurance and supplier production acceptance remain open; no endorsements or qualification claims.

## Explore and integrate

[Project site](https://susmithmrm.github.io/ZARC/) · [Vehicle architecture](https://susmithmrm.github.io/ZARC/modern-architecture.html) · [Partner guide](https://susmithmrm.github.io/ZARC/platform-adoption.html) · [Architecture index](docs/architecture/00-README.md) · [Diagrams](docs/architecture/diagrams/README.md)

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

## License

[MIT](LICENSE). Third-party dependencies retain their respective licenses. No compulsory ZARC-specific runtime royalties; selected vendor dependencies may have separate terms.
