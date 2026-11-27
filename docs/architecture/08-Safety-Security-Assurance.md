# Z08 Safety security and assurance concept

## Scope and responsibility

No IEC 61508 safety integrity level (SIL), automotive safety integrity level (ASIL), regulatory or production qualification is claimed. In verification documents, SIL means software-in-the-loop instead. Zephyr's safety initiative describes a limited-scope certification objective [S03](13-References.md#s03); that does not establish ISO 26262 qualification for ZARC or its selected drivers, compiler, hardware and product. The OEM owns item definition, hazard identification, risk classification, safety goals and vehicle validation. Safety manager confirms applicable ISO 26262 edition, including published changes; the November 2026 ISO update lists draft revisions [S23](13-References.md#s23). A platform evidence bundle may support a specific integration under assumptions of use; it is not a blanket safety rating.

## Safety architecture (D16)

Establish development interface agreement, HARA, functional/technical safety concepts, allocation of functions and FTTI, safe states/reaction, independent monitoring and hardware architecture. Derive SW safety requirements, mechanisms, traceability, verification and analyses (FMEA/FTA or justified method). Analyze common cause and freedom from interference for memory, timing, resources, communications and power/reset. If isolation or independence is insufficient, separate implementations or exclude that configuration. Safety reaction is function-specific: power removal can itself be unsafe. No generic fallback or reset timer is prescribed. Begin with QM lab demonstration; future assessment is contingent on agreed scope, supported hardware and independent evidence.

| Failure family | Architectural candidate | Evidence needed |
|---|---|---|
| Stale/unavailable host | Expire remote intent; local allocated behavior and freshness | HIL outage/reconnect and state assertions |
| Incorrect/late local work | Deadline/logical supervision, approved degraded response | Timing analysis, fault test, requirement allocation |
| Incorrect feedback | Plausibility, quality and independent monitor as required | Measurement/HW failure analysis |
| Unexpected reset/brownout | Reconciliation, retained cause, verified startup | Reset/brownout HIL and hardware facts |
| Shared resource interference | Partitioning and budgets, separation if unproven | Memory/temporal/DMA analysis and stress |
| Failed update | Compatible recovery path, verified eligibility | Interrupted update and version skew |
| Network/device loss | Contained endpoint fault domain | Multi-endpoint HIL and timing analysis |

These are illustrative fault families, not completed HARA or FMEA. Allocated safety limits are OEM-specific.

## Security concept (D17)

Assets include physical authority, identity and keys, firmware configuration, diagnostic integrity, availability and selected telemetry. Trust boundaries: vehicle/IVI/external communications, host broker, MCU transport, privileged RT execution, boot/recovery, hardware and engineering supply chain. OEM/Tier 1 perform TARA, define cybersecurity goals, control allocation, verification and risk acceptance; ISO/SAE 21434 is a lifecycle standard [S21](13-References.md#s21). Use vetted authentication/integrity and freshness mechanisms appropriate to board/transport, secure boot/update, least privilege, service role policies, memory/resource bounds, network segmentation, protected credential provisioning, controlled debug/maintenance, incident and vulnerability response. Do not expose keys or arbitrary low-level device controls to guests. Independent safety E2E protects allocated accidental error model and must be separately analyzed; cryptographic integrity does not replace safety analysis.

## Software update and recovery (D18)

Vehicle update coordinator manages inventory, signed metadata and compatible deployments. Uptane Primary/Secondary model is candidate [S15](13-References.md#s15); MCUboot provides selected verified boot/test/revert schemes [S05](13-References.md#s05). Analyze flash partition/boot choice and recovery guarantee per board. Stage and verify image/metadata, hardware and configuration compatibility, authorized eligibility, storage and power conditions; boot candidate, run agreed self-tests, confirm only after compatibility/health criteria. Interruption or rejection uses approved recovery path. Anti-rollback counter policy and allowed recovery must be jointly designed, with no irreversible progression before recovery viability. Distinguish application image, bootloader, config and companion host contract versions. Confirm compatibility across paired host/MCU releases; no silent downgrade. Capture version manifest, exact signed release, evidence and update outcome. ISO 24089 scope and UNECE R155/R156 OEM obligations inform supplier work [S22](13-References.md#s22), [S24](13-References.md#s24).

## Assurance case

Top claim (unproven): specified ZARC configuration satisfies allocated functional, timing, safety and cybersecurity requirements within documented assumptions. Subclaims: local authority preserved; execution/resources bounded; fault responses correct; interfaces compatible; update/recovery viable; risks managed; lifecycle controls and evidence sufficient. Evidence links requirement → allocation/design → code/configuration → verification/test → measured result → review → released configuration. Review residual risks, assumptions and limitations with responsible integrator. Deliver safety plan, DIA, safety requirements, assumptions of use, safety manual when substantiated, analyses, measured timing, tool confidence, change impact, verification results and independent assessment as appropriate. Cybersecurity plan, TARA, concept, verification, SBOM, incident response, update and residual risk acceptance. Automotive SPICE process assessment is a future organizational activity, not achieved.

## Assurance boundaries and evidence integrity

Safety analysis must cover entry to bootloader, startup pin defaults, controller reset, update and shutdown as well as runtime. Assess independence, common-cause and dependent failures for shared power, clock, bus, memory, watchdog, drivers and software; no reliance on a diagnostic mechanism with common failure absent justification. An external independent monitor is required only where allocated by system safety analysis. Define safety-related data integrity/freshness mechanisms by fault model and residual error analysis, not by generic CRC or authentication. Document measurement and verification method, test environment and single-fault assumptions where applicable. The release decision must clearly distinguish unassessed QM functions and separately assessed configurations.

Security review must include supply chain provenance and SBOM, boot trust and protected provisioning, rollback/recovery, authority delegation, data privacy, production debug and incident handling. Safety and security policy can conflict (e.g. recovery); resolve through joint risk review before vehicle deployment. No unsupported claim of secured or qualified configuration. Supplier evidence requires scoped assumptions, declared gaps and accepted residual risk; ISO/SAE and regulatory applicability belongs to integrator/legal review.
