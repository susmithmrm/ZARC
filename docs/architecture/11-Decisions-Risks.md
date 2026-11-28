# Z11 — Architecture decisions and open risks

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase.

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
