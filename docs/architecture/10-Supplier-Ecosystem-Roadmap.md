# Z10 — Global supplier adoption and lifecycle

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase.

## Partner model
OEM owns vehicle allocation, architecture and acceptance; Tier 1 owns integration; Tier 2 supplies independently versioned components; silicon vendor owns BSP and driver evidence; Tier 3 benefits indirectly; AI and tools vendors provide optional bounded integrations. Open MIT core, explicit third-party terms, private customer product inputs.
## Customer integration
Publish pinned SDK, component exchange package, interface and conformance fixtures, exact target support matrix, evidence and known limitations. Demonstrate independent replacement and two customer variants without a mandatory core fork. Customer requirements and quality obligations are controlled product allocations linked to Z12, not extra platform baselines (SUP-017). Assess contracted IATF customer-specific requirements, APQP, PPAP or VDA 2, FMEA, 8D, field quality and change notification. Regional regulatory applicability belongs to the exact product and jurisdiction. No platform certification claim.
## Lifecycle
Version and compatibility policy, reproducible archive, patch/backport and migration route, accountable maintenance owner and explicit support funding. Quarterly public industry roadmap review; annual competitor and standards review; review before major changes. Publish gaps and dispositions (SUP-015).
## Implementation sequencing
1. Native composition, component descriptors and engineering graph.
2. Authorized observation and identity reconciliation.
3. Deployment-unit management, state migration and recovery.
4. Multi-vendor physical targets, legacy and zonal interoperability.
5. Optional AI and heterogeneous providers.
6. Customer pilots, measured economics and scoped qualification.
Portfolio adoption review ADOPT-082 includes both architecture generations. No partnerships or superiority claims without independently substantiated evidence.

## Supplier engineering obligations
SUP-001–004/006/011/014–018 and ADOPT-035–044/080–082 allocate contributions, maintenance, migration, IP boundaries, tool exchange, continuous alignment, licensing, component exchange, reproducible SDK, localization, quality processes, economics and independent partner evaluation. The common core must be integrable without compulsory proprietary tooling or ZARC-specific runtime royalties; selected vendor SDK and third-party license restrictions remain explicit. Public target maturity is derived from PL-010 under ADOPT-043. Compare equivalent costs and effort using VER-017, including negative outcomes and missing evidence.
## Governance and support
Maintain decision authority, accountable owners, conflict-of-interest disclosure, escalation and succession, maintenance window, response and backport policy, dependency end-of-life and reproducible archive. A single maintainer is a dependency, not independent approval. Use the contribution guide and responsible independent reviewers. Customer-specific deliverables are controlled allocations; do not publish confidential material. Industry research is a recurring design input, not evidence of procurement or partnership.
