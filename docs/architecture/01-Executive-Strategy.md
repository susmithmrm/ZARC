# Z01 — Strategy: one foundation for two generations

> **ZARC 2035 | Design baseline candidate | 28 November 2026.** The single normative-candidate register is [Z12](12-Requirements-Verification.csv) (267 requirements). This design describes the intended architecture, not implemented or qualified functionality. See [implementation evidence](../IMPLEMENTATION-STATUS.md).

## Mission
Make ZARC the preferred open-source real-time platform for OEMs, Tier 1 and Tier 2 suppliers, semiconductor manufacturers and intelligent embedded developers by 2030–2035, while remaining compatible with distributed CAN/CAN FD/LIN ECUs. Preference must be earned through independent comparisons and actual adoption.

| Stakeholder | Outcome | Proof |
|---|---|---|
| OEM | Topology freedom, long-term upgrades and cost control | Legacy and zonal portfolio, lifecycle cost |
| Tier 1 | Reuse across customer programs | Independent component exchange and customer variants |
| Tier 2 | Small intelligent components | Minimal native build and SDK |
| Silicon | Native hardware integration | Two physical MCU vendors, exact support matrix |
| AI and embedded | Optional inference and portable contracts | Bounded reference and benchmarks |
| Quality and assurance | Reviewable evidence | Scoped product acceptance |

## Competitive approach
Compete on necessary engineering outcomes, not copying AUTOSAR layers. Respect licensed specifications; complement S-CORE, AAOS, Linux and QNX. Target 30% reusable platform effort and 25% lifecycle cost reductions as hypotheses (ADOPT-042); report negative results. No vendor endorsement or universal compliance.
## Architecture principles
One codebase, one Z12, native Zephyr first. Independently maintained micro-components are not mandatory dynamically loaded microservices. Every selected capability has resource, authority, lifecycle and evidence allocations. Hardware control stays local; management and AI are bounded. Preserve customer IP and avoid mandatory core forks.

## Retained engineering detail
Existing design detail is retained for compatibility and trace continuity. Where wording differs, Z12 and the current architecture above govern. Historical examples are not evidence of new functionality.

## Mission and proposition

ZARC should become the preferred engineering shortlist for automotive real-time controllers where a code-first, upstream-first architecture meets the OEM's requirements. Its product is not an RTOS, generic middleware or AUTOSAR clone; it is intended to become an integration and evidence foundation for physical control. The controller retains authority for its allocated devices and local timing. Central compute supplies desired operating intent, services and orchestration, not direct hardware ownership.

ZARC's differentiation must be measured against customer baselines: porting effort, integration time, application footprint, latency and jitter, independent operation, compatibility, maintainability, evidence completeness and cost of long-term support. Claim none without comparative evidence. The initial target audience is a Tier 1 or OEM program with flexible QM companion requirements, not a safety-critical mass-production replacement.

## Future-proof design hypotheses

FACT: BMW publicly describes centralized high-performance computers with zonal architecture [S20](13-References.md#s20). FACT: AOSP SDV describes isolated compute instances, service bundles, SOME/IP transport and power management [S10](13-References.md#s10), [S11](13-References.md#s11), [S12](13-References.md#s12). INFERENCE: both centralization and distributed autonomy may coexist; some edge endpoints may lose their local MCU while others retain it. The architecture must support external controllers, zonal controllers and integrated real-time islands. No inference about proprietary OEM source, processor selection or procurement intent follows from public announcements.

| Trend | ZARC response | Validation |
|---|---|---|
| Centralized computing | Host broker and multi-VM adapters; physical autonomy remains local | Guest restart and link-loss HIL |
| Zonal architectures | Configurable endpoint/vehicle networking; bounded failure domains | Representative traffic and endpoint outage tests |
| Integrated RT islands | IPC plus resource ownership and reset-domain gates | SoC vendor isolation evidence |
| Remote simple endpoints | Upstream controller manages passive/remote I/O | Electrical fallback and network timing analysis |
| Independent software updates | Versioned contracts, compatible manifests, recoverable updates | Interrupted update/version skew tests |
| Supplier ecosystem | Board, product and service contracts with independent conformance | Second-vendor and second-supplier integration |
| Emerging service platforms | Boundary adapters; stable semantic contract | Cross-stack interoperability |

## Competitive and ecosystem positioning

Commercial AUTOSAR Classic stacks offer a mature standardized ecosystem and established product/qualification evidence; ZARC does not replace these obligations. For flexible code-first programs, compete through simpler composition and transparent evidence. Zephyr alone supplies RTOS and device APIs; ZARC adds automotive domain services, integration and assurance. Eclipse OpenBSW overlaps with embedded automotive software and is a reuse candidate [S16](13-References.md#s16). LIBERSOM is an incubating embedded SOME/IP project [S17](13-References.md#s17); measure and review before selection. S-CORE and OpenSOVD are complementary high-performance/diagnostic ecosystems [S18](13-References.md#s18). Proprietary BSPs may be necessary for particular silicon; keep such dependencies explicitly scoped and supported.

## Branding and commercialization

ZARC stands for Zephyr Automotive Real-Time Control. Tagline: The Zephyr-native real-time control platform for software-defined vehicles. It is a selected working project name. Branding, trademark, domain and namespace clearance remain open; no official Zephyr or Eclipse affiliation, endorsement or certification is claimed. Publish architecture, interoperability contracts, conformance suite, example and reference targets. Original ZARC source and associated documentation are licensed under [MIT](../../LICENSE); preserve third-party notices and evaluate dependency compatibility. Fund integration, extended maintenance, certified-scope evidence services only when support obligations and capability are real.

## Product success measures

G1: reproducible portable builds, native target build, host simulator, one benign controller use case and documented timings. G2: second board from a different vendor without generic service modifications, smart edge configuration and interrupted update test. G3: one credible AAOS/SDV or Linux host integration, SOME/IP interoperability and integrated RT feasibility. G4: actual design partner and scoped evidence agreement. G5: frozen configuration, reviewed evidence and sustainable release/support. Stop or pivot if generic platform value is not demonstrated, customer requirements mandate incompatible assurance scope, or long-term maintenance funding is absent.
