# Z09 Verification and engineering infrastructure

## Verification pyramid (D19)

Unit: policy transitions, pure logic, native driver contracts and bounded serialization fixtures. Native simulation/SIL: product variants, lifecycle, compatibility, restart, event behavior. Renode or appropriate simulator: board-specific models where fidelity justified; documented limitations and comparison to hardware. HIL: real benign target electrical feedback, UART/network/IPC, independent reset, resource/timing and update interruption. Vehicle integration/validation: use cases, real hardware/environment, EMC, aging, safety requirements and OEM acceptance. Do not infer physical, timing or environmental evidence from simulation alone. Zephyr ztest/Twister and pytest harness are candidates [S02](13-References.md#s02).

## Verification matrix and test record

Requirements register Z12 carries stable ID, parent objective, applicable profile, normative-proposed statement, design reference, test ID, method, acceptance, owner, gate, status and evidence. Requirement IDs are never recycled. Every requirement maps to at least one test/review; unsupported optional profiles are marked not selected by approved applicability decision, not passed. Capture exact source revisions, board/stepping, build/config digest, toolchain, scenario, environment, workload, procedure, expected/actual, raw trace, verdict, defect, reviewer and date. Requirement closure requires accepted evidence, not source presence. Trace changes and run affected regression. Review measurement uncertainty and test representativeness.

## Test families and coverage

| Family | Evidence |
|---|---|
| Architecture | Dependency direction, profile closure, resource ownership, unsupported capability handling |
| RTOS | Context, deadline, latency, stack, memory, queue, lock and interrupt tests/analysis |
| Service | Initialization failure, rollback, readiness, quiesce/resume, recovery |
| Contract | Golden request/event fixtures, version mismatch, stale state and idempotence |
| Host | Guest independence, broker sole ownership, typed adapter conformance |
| Target | Build, native device and actual target porting; physical timing and HIL |
| Update | Authenticated approved image, version skew and interruption recovery |
| Assurance | Allocated fault-response, risk controls, regression and independent reviews |

Set coverage and static-analysis/deviation targets by approved safety plan and product scope. No destructive testing on operational vehicles. Test denial and failure handling only in authorized isolated test environments.

## Build and release engineering

The current desktop CMake build has CI jobs for Windows and Linux in Debug and Release. Exact compiler versions and upstream Zephyr dependencies remain to be pinned. For embedded release builds use a pinned west manifest, toolchain, native CMake/Kconfig/devicetree/sysbuild. Maintain CI build matrix across selected board/profile/feature/compiler options; run unit and contract fixtures, native_sim, applicable simulator and HIL. Enforce formatting, warnings, static analysis, reproducibility, license review and SBOM. Build metadata records commit, dependencies, config hash, map/resource report and binary hashes. Release artifacts are signed, versioned and reviewed. No bypass of target evidence gate. Fail closed on missing requirements, incompatible dependencies, unaccepted deviation or evidence gap. Maintain vulnerability triage, controlled backports, update compatibility and change-impact matrix; publish supported configuration list and end-of-support plan.

## Stage gate evidence

| Gate | Acceptance |
|---|---|
| G0 | Architecture baseline, owners, benign scope and desktop contract allocations; select boards before native bring-up |
| G1 | Desktop behavior and two-process integration first; then native MCU demo, pinned build, target contract tests and nominal resource/timing evidence |
| G2 | Second vendor/edge profile, HIL and update interruption, documented limitations |
| G3 | AAOS/SDV/SOME-IP or relevant host adapter, VM isolation and IPC feasibility evidence |
| G4 | Design partner pilot, hardware/product requirements, safety/cyber scope and independent gap review |
| G5 | Reviewed exact configuration, allocated requirements/evidence, accepted risks, release/support/manufacturing readiness |

G1–G3 are engineering gates, not vehicle approval. G5 release readiness does not automatically imply qualification; scope and authority define the claim.

## Requirement allocation and reproducible evidence gates

Each objective (OBJ-SYS, OBJ-RT, OBJ-PL, OBJ-SVC, OBJ-CON, OBJ-HOST, OBJ-NET, OBJ-ASR, OBJ-VER, OBJ-SUP) is defined in Z14. A row Profile=ALL is applicable at the configuration's own admission gate; lab-only thresholds have G1-C-LAB scope, not all boards. SELECTED is evaluated against an explicit capability selection; optional unavailable functions require a recorded applicability rationale. C/Z/I indicates only configurations with a selected host adapter, not all controllers. Network/IPC gates distinguish Ethernet versus integrated and zonal endpoint tests. A gate value is earliest planned verification, not automatic evidence inheritance. Preserve original IDs; clarified requirements carry revised statements and new test/owner data as appropriate. Test record must link original objective, design, verification and result; one verification ID may have multiple target-specific instances. Define measurement setup, load, sample population, limits, uncertainty and clock source before execution. Add explicit review/test for each architecture finding; keep failures and unverified hazards open until responsible owner accepts evidence.
