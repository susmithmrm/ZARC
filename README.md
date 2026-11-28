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

**Phase:** requirements and architecture. Implementation and verification are planned next. The single 267-entry [Z12](docs/architecture/12-Requirements-Verification.csv) defines required outcomes. See [implementation plan](docs/IMPLEMENTATION-STATUS.md) and [derived trace](docs/REQUIREMENT-TRACE.csv). No product qualification or partner endorsement is claimed.

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

These commands are development-environment entry points, not acceptance evidence. The implementation phase will assess the retained source against Z12. No physical vehicle hardware is authorized by this quick start. See the [implementation plan](docs/IMPLEMENTATION-STATUS.md).


## License

[MIT](LICENSE). Third-party dependencies retain their respective licenses. No compulsory ZARC-specific runtime royalties; selected vendor dependencies may have separate terms.
