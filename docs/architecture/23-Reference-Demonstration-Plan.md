# 23 — Reference demonstration and verification plan
Prerequisite: approved low-voltage bench, pinned builds, wiring and output disable checked. No tests executed yet. All acceptance limits provisional until profile allocation.

| Test | Action | Expected result / evidence | Trace |
|---|---|---|---|
| D01 | Cold boot each MCU | readiness only after checks; timestamp boot and first service | SYS-006, SVC-003, ADOPT-048 |
| D02 | Toggle benign indicator | accepted intent distinct from measured feedback | SYS-004, CON-007 |
| D03 | Move target near ToF | live distance and quality; record raw sample | ADOPT-002 |
| D04 | Rotate wheel/encoder | valid speed/angle, units and timestamp | ADOPT-002 |
| D05 | Cover light sensor | local lamp policy responds with feedback | SYS-001, ADOPT-003 |
| D06 | Warm sensor safely | fan response within allocated threshold and time | RT-009, ADOPT-003 |
| D07 | Open door switch | twin displays correct node and signal | ADOPT-046 |
| D08 | Disconnect benign sensor | invalid/stale quality, no fabricated value | CON-009 |
| D09 | Restart host | local allocated behavior continues, fresh session | RT-004, HOST-002 |
| D10 | Disconnect bench CAN node | independent endpoints remain within budgets | NET-004 |
| D11 | Reconnect node | new epoch and reconciled snapshot | CON-015 |
| D12 | Apply approved nominal traffic | measure queue high-water and latency | RT-013, CON-013 |
| D13 | Cycle low-voltage power | record reset cause and recovery | SYS-007 |
| D14 | Approved benign firmware update | validated image and recovery evidence on supported target | ASR-006/007 |
| D15 | Repeat common scenario on second vendor | same semantic contract, documented differences | PL-006 |
| D16 | Run 24h benign load | deadline misses and resource high-water reported | RT-010/012 |
| D17 | Activate physical output disable | actuator supply removed independently | bench safety review |
| D18 | Reproduce build from clean environment | pinned inputs and artifact identity recorded | VER-002 |

## Presentation sequence (10 minutes)
1. Show transparent 3D sedan and physical controller map (1 min).
2. Demonstrate real wheel, door, distance and light sensors (2 min).
3. Demonstrate local lamp/fan and independent feedback (2 min).
4. Show network and host outage/recovery (2 min).
5. Show same contract on alternate MCU and profile footprint (2 min).
6. Show evidence dashboard, limitations and roadmap (1 min).

## Evidence template
Test ID, full requirement IDs, target serial/revision, firmware hash, Zephyr/toolchain version, manifest hash, wiring revision, instrument/timebase, conditions, expected/actual, raw trace path, result PASS/FAIL/BLOCKED/NOT RUN, reviewer, date. Simulator and physical evidence separate. No inferred PASS. Record median, p95, p99, maximum observed latency and deadline misses; observed maximum is not WCET. Publish flash/RAM/stack, queue high-water, CPU and power measurements per profile. Existing G1 provisional 10 ms benign local deadline and 20% memory headroom remain applicable only to their allocated lab profile.

## Exit gates
A: one physical benign indicator with feedback; B: two physical nodes and local autonomy; C: three nodes, sensor suite and live twin; D: independent second-vendor evidence and repeatable SDK; E: separately scoped advanced automotive networking and integrated RT. No gate implies roadworthiness, ISO 26262 or production cybersecurity qualification.
