# 21 — Architecture simplicity guardrails
Proposed architecture decision; existing safety and authority requirements remain binding when allocated.

## Runtime rule
One native Zephyr application per MCU, one composition root, one owner per physical output, one typed contract per service. Core provides admission, lifecycle and health only where needed. Services are optional linked modules. Connect is optional; Host is external. Platform is native DTS/Kconfig and thin domain adapters. SDK and Assurance are build/test activities, not runtime layers. Product policy is configuration plus small application code. No general RTOS facade, plugin loader, runtime dependency resolver, universal hardware wrapper or mandatory broker on standalone edge. Use static composition and native drivers. No duplicate scheduler, bus, storage or network stack.

| Profile | Required runtime | Optional |
|---|---|---|
| Edge | Core, product, native device | Connect, storage |
| Companion | Core, power/supervision, selected link | Ethernet, update |
| Zonal | Core, endpoint services, selected link | LIN, power distribution |
| Integrated | Core, native IPC, allocated device | Host adapter outside RT domain |

## Review gates
- New abstraction requires two concrete consumers or a documented isolation/portability need.
- New feature must be removable by build configuration; absence must not silently remove required protection.
- One new sensor should require board configuration and provider/product mapping, not edits to Core or Connect.
- No mandatory heap on allocated hard RT paths; measure stack, flash and RAM by profile.
- Maintain an explicit dependency graph and reject cycles, board-specific core imports and duplicated ownership.
- Measure clean build, onboarding time, code size and timing before/after each feature; regressions require rationale.
- Preserve typed intent, independent feedback, bounded execution and safe bench defaults. Simplicity is not a waiver of evidence.
- Adopt upstream Zephyr functionality rather than maintaining forks wherever feasible.
- Use optional standard adapters; do not require AUTOSAR, SOME/IP, DDS or gRPC inside all MCUs.
- Review new adoption requirements against existing baseline; resolve duplicates and allocate by product before promotion.

Acceptance: independent developer builds a minimal edge sample, adds a sensor without changing generic core, and demonstrates same service contract on second target. Report actual commands, changes, resource budget, limitations and evidence. This document does not claim these tests have passed.
