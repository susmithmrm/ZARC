# Z02 — System architecture and component contracts

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Topology and logical planes
Support standalone edge (E), companion (C), zone (Z) and integrated real-time (I). Distributed, domain/hybrid and centralized architectures coexist. Small ECUs need no host, cloud, AI or Ethernet.
The control plane owns allocated I/O, deterministic behavior and independent fault response. The observation/management responsibilities include inventory, diagnostics, trace and update coordination. The required visibility contract can use local extraction; selected network and fleet providers are optional. Management does not own physical I/O or block allocated control. Central AI/HPC provides authorized typed intent or advice only.
## Micro-components
Descriptor: stable identity, release, typed ports and interaction type, dependencies, trigger, timing, CPU/memory/queue budgets, lifecycle, criticality, fault disposition, state schema, owner and access class. Composition resolves ports and native bindings at build time. Unsupported placement or resource conflict fails admission. Independent components may share a linked image without being separately loadable. Replacement requires conformance and no unrelated core edits (SYS-011, SUP-018).
## System graph
SYS-018 links descriptors, deployment units, resources, dependencies, topology, contracts and evidence. Native devicetree/Kconfig/manifest remain authoritative for their own facts; each derived fact carries source/version. PL-024 reconciles intended, configured, built, deployed and observed identities. No second hardware database.
## Authority and interactions
Single owner or approved arbitration through boot/reset/maintenance. Requested state differs from actual feedback. Unknown/stale are not healthy. Redundant network does not imply redundant hardware ownership. Local lifecycle independent of host, VM and vehicle mode. Latest-value, queued event and request/response specify quality, version, lifetime, delivery and failure; placement preserves semantics, not identical timing. Product allocation governs timing. D01–D08, D10, D23–D25.

## Allocation and admission
Each product defines independent mode authority for vehicle, local controller, host session and service. The composition root owns dependency resolution; application components never import private vendor drivers. Explicitly allocate reset, boot, clock, memory, DMA, bus, physical output and maintenance ownership. Cross-controller requests carry typed intent, correlation, expiry and observed quality. Recover by reconciling actual state rather than replaying stale commands. A selected optional dependency must not make an independent required function unavailable.

## Design allocation and integration invariant
| Scope | Z12 allocation |
|---|---|
| Autonomous operation and physical authority | SYS-001–008, SVC-013 |
| Minimal core, accelerator authority, reusable components | SYS-009–012, SYS-017 |
| Topology, failover, system graph | SYS-014, SYS-015, SYS-018 |
| Product compatibility and deployment identity | PL-024, PL-025 |
The mandatory architecture graph and authorized visibility obligations apply to all admitted configurations. A networked management service is optional; a standalone image can use approved local extraction. Each component is independently versioned, even when the deployment unit is a statically linked whole image. Do not promise independently executable or hot-replaceable modules on every MCU. A redundant control configuration needs an explicit approved ownership transfer, unavailable-state policy and evidence; redundancy is not implicit in the topology.
