# Z08 — Assurance, authorized visibility and continuous update

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Scope
OEM owns item, hazard analysis, safety goals, vehicle validation and type approval. Allocate DIA, HARA, FTTI, TARA, assumptions of use, independence, safety manual and residual risk. ISO 26262, ISO/SAE 21434, ISO 24089, UN R155/R156, ISO 21448 and ISO/PAS 8800 apply to allocated scope. No platform ASIL, SIL, cybersecurity, regulatory or production claim. Upstream certification is not automatically inherited.
## Boundaries
Analyze startup/reset/shutdown/update, shared resources, common-cause faults and independent protection. Local authority and bounded execution survive allocated failures. Access classified by role/object; expose status, not secrets or arbitrary low-level control. Approved security and boot providers; no proprietary crypto. Safety communication error model separate from cybersecurity. Incident, dependency, privacy and key lifecycle controls.
## Update and recovery
Every selected deployment unit has identity, compatibility, eligibility, activation and recovery policy. Verify authenticity, target and dependencies before activation; preserve state schema. Stage, activate in eligible mode, verify observed health/compatibility, confirm or recover; report actual versions. Firmware, model, configuration, boot and security epoch distinct. Anti-rollback and authorized recovery compatible; multi-node update includes offline and mixed versions. Vendor/service replacement documented when update impossible. Uptane/MCUboot/sysbuild integration candidates. Physical and independent verification remain external.

## Evidence allocation
Assess exact product assumptions and fault tolerant time intervals; independent hardware protection and product-specific safe behavior are externally allocated. Verification must cover allocated failure and recovery, interference and actual physical evidence. Security policy and safety mechanism claims are distinct. Report missing vendor, hardware, licensed standards and independent assessment as open. No standards conformity is implied by architecture or simulator success.
