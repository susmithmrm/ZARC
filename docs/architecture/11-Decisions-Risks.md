# Z11 — Architecture decisions and open risks

> **ZARC 2035 | Proposed design baseline | 28 November 2026.** The single normative-candidate register is [Z12](12-Requirements-Verification.csv) (267 requirements). This is not evidence of implementation or production qualification.

## Decisions
| Decision | Evidence allocation |
|---|---|
| Native Zephyr and portable domain logic | RT-001 |
| Static composition by default | SYS-010/011/018 |
| Authorized bounded observation plane | SVC-028/029, ASR-029 |
| Deployment units, not mandatory hot loading | PL-025, ASR-030 |
| Reconcile designed, configured, built, deployed and observed identities | PL-024, VER-024 |
| Optional bounded advisory AI | ADOPT-054 |
| One Z12, derived trace and allocation | VER-008/023 |
| Scope-specific evidence, not brand-level support | PL-010 |

Open risks: physical two-vendor ports, real-time resource budgets, executable selection rules, licensed standard access, independent peer conformance, component exchange, state migration, authorized introspection, customer acceptance, independent qualification and maintenance funding. Each remains open pending accountable evidence. No universal dynamically loadable architecture or safety reaction is assumed.

## Retained engineering detail and historical context
Existing design detail is retained for trace continuity. The current Z12 and preceding architecture govern where wording differs; historical counts, findings, examples and acceptance claims are scoped to their original revision.

## Decision log

| ID | Status | Decision/question | Owner/gate |
|---|---|---|---|
| ADR-001 | Proposed | Native Zephyr execution; no parallel general RTOS abstraction | Architect G0 |
| ADR-002 | Proposed | Local physical authority and typed desired intent | OEM/system G0 |
| ADR-003 | Proposed | Static product/profile composition; build fails on invalid dependencies | Embedded G1 |
| ADR-004 | Open | Zephyr release/LTS, SDK, compiler and maintenance plan | Release G0 |
| ADR-005 | Open | First two vendor boards, hardware capabilities and BSP support | Vendor/Tier 1 G0 |
| ADR-006 | Proposed | One semantic contract, configurable transport providers | Connect G0 |
| ADR-007 | Open | UART framing/codec, delivery semantics and board ICD | Connect G1 |
| ADR-008 | Open | Approved authentication/identity provisioning and channel profile | Security G1/G4 |
| ADR-009 | Proposed | Trusted host broker is exclusive physical link owner | Host G1/G3 |
| ADR-010 | Open | AAOS branch, VHAL property scope and SDV service mapping | OEM/Host G3 |
| ADR-011 | Open | SOME/IP stack, service identifiers and independent peer | Connect G3 |
| ADR-012 | Open | Integrated RT reset, clock, resource isolation and IPC backend | SoC vendor G3 |
| ADR-013 | Open | Safety item definition, goals, FTTI, allocated reaction | OEM/safety G4 |
| ADR-014 | Open | Boot/update image layout, recovery and counters | Embedded/security G2 |
| ADR-015 | Open | Required diagnostics, DTC, SOVD and fleet interface | OEM G3/G4 |
| ADR-016 | Partially resolved | MIT license selected; trademark/domain and supplier agreement open | Product/legal G0 |
| ADR-017 | Open | Native isolation feasibility and mixed-criticality admission | Safety/SoC G3/G4 |
| ADR-018 | Open | Long term support SLA, release schedule and end of support | Product G4 |
| ADR-019 | Open | Zonal endpoint topology and electrical safety allocation | OEM/Tier 1 G4 |
| ADR-020 | Proposed | No production support without exact configuration evidence | Assurance G0 |

## Risk register

| ID | Risk | Likelihood/impact | Mitigation / trigger |
|---|---|---|---|
| RK-01 | Vendor BSP unsupported or incomplete | High/high | Select two candidates and document errata; no board claim before evidence |
| RK-02 | Generic architecture grows into redundant RTOS | Medium/high | Interface review, direct native mapping, code purity checks |
| RK-03 | Safety scope exceeds evidence | High/high | QM first; item allocation and independent gap review |
| RK-04 | Resource/latency budget missed | Medium/high | Instrument early, compare target profiles, reduce optional features |
| RK-05 | Shared SoC faults defeat independence | High/high | Require isolation/reset evidence; external companion fallback |
| RK-06 | Host/MCU compatibility drift | Medium/high | Single schema, generated mapping and paired version tests |
| RK-07 | Supplier-specific forks diverge | Medium/high | Contribution contract, upstream-first and release policy |
| RK-08 | Update failure or incompatible image | Medium/high | Verified recovery, interrupted updates and compatibility matrix |
| RK-09 | Remote endpoint enlarges fault domain | Medium/high | Endpoint containment, approved local hardware protection and timing |
| RK-10 | Security provisioning inadequate | Medium/high | TARA, protected key lifecycle and independent assessment |
| RK-11 | Third-party maturity/license gaps | Medium/high | Evaluate upstream fitness; pin versions; counsel review |
| RK-12 | No customer differentiator or support funding | Medium/high | Partner interviews, baseline comparison; G2 go/no-go |
| RK-13 | Name/brand conflict | Unknown/medium | Clearance and branding review before public launch |
| RK-14 | Evidence not reproducible | Medium/high | Manifest-bound trace, raw artifacts, fail-closed release gate |

## Architecture review checklist

At each gate confirm ownership, supported profiles, actual dependencies, changed interface versions, resource/timing evidence, assumptions of use, unresolved requirements, residual risks, maintenance and supplier responsibilities. Record approval or rejection against exact version. No undocumented deviations in released builds.
