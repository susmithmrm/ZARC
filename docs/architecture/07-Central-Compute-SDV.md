# Z07 — Central compute, host and fleet integration

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Host boundary
Trusted broker exclusively owns physical link where selected. Guest, host, controller and service lifecycle independent. Scoped principals submit typed intents; local owner validates. Quotas and reserved resources protect independent functions. Restart clears stale validity and reconciles fresh snapshots; no replay of expired intent.
## Ecosystem
Selected AAOS VHAL, S-CORE/SDV, SOME/IP, gRPC, Linux/QNX or hypervisor adapters preserve semantics; host adapters replaceable independently. VHAL AP_POWER_STATE_REQ (VMCU→Android), AP_POWER_STATE_REPORT (Android→VMCU); pin branch and validate direction and states. Guest readiness is not physical readiness. Shared clock, boot, memory, device and DMA ownership need target evidence.
## Fleet and AI
Fleet coordinator/cloud are external optional integrations. Report actual installed identity, freshness, health, staged/active/recovered status and compatibility. Policy-defined rollout pause belongs to coordinator; activation only in eligible mode. AI enters authorized advisory boundary; heavy perception stays on suitable processor. No cloud requirement for local control.

## Ownership and independent lifecycle
A host or guest restart does not imply controller reset; link epoch and controller boot epoch are distinct. Preserve independent allocated functions across loss and reconnection. Guest readiness is not electrical or physical readiness. Host, vehicle and local mode transitions require allocated eligibility, bounded coordination, observed outcomes and an approved response to unavailable evidence. Host adapters translate only authorized semantic contracts; the real-time controller retains physical authority.
