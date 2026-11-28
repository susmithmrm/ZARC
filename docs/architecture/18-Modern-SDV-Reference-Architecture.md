# ZARC — Distributed to intelligent SDV reference architecture

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase.

## Four placements, one foundation
Standalone distributed ECUs over legacy networks; domain/hybrid controllers; central compute with zones and edge nodes; heterogeneous central AI with real-time islands. Common component descriptors, typed contracts, native configuration and resource authority remain invariant. Required local or selected external observation and management responsibilities reconcile inventory, health, correlations and deployment units without blocking local control. The sedan visualization is an illustrative, read-only design view, not a production design or actual measured vehicle topology.

## Four architecture scenarios
| Scenario | Allocation | Required portfolio evidence |
|---|---|---|
| Standalone legacy ECU | Edge and local function; optional host | Conventional CAN, CAN FD and LIN independent peer fixtures |
| Domain or hybrid | Selected domain/companion and conventional controllers | Typed legacy coexistence, independent local behavior |
| Centralized and zonal | Central applications and at least four physical controllers for the selected portfolio reference | Physical topology, independent failure domains and equivalent allocated semantics |
| Heterogeneous real-time island | Separate RT domain with selected IPC and accelerator | Exact reset/clock/memory/authority isolation evidence |

The preceding cases define validation scenarios, not a mandatory vehicle topology or a fixed bill of materials (ADOPT-001/022/044). Local visibility, deployment-unit policy and versioned contracts apply across admitted configurations; networked fleet services and AI remain optional. The actual accepted scope is configuration-specific.
