# Repository audit — 27 November 2026

Scope: current desktop prototype, existing architecture and public repository. Review is an engineering baseline, not production validation.

| Area | Result |
|---|---|
| Git integrity | git fsck passed, main tracks origin; E: ownership exception required |
| Remote metadata | public, main default, Pages enabled, issues and projects enabled; wiki/discussions disabled |
| Governance | main is unprotected; MIT license selected. Owner must choose contribution policy and desired merge rules before enforcement. No external access granted. |
| Architecture | 111 proposed requirements and desktop allocations, 28 review findings and 22 diagram IDs checked for consistency. Full device, service registry and broker implementation remain open. |
| Source | Fixed old request ID reuse in single-session prototype; documented retention and deadline semantics. Added boundary tests. |
| Build | Warnings are errors for all C targets; Windows/Linux build presets and VS Code tasks separated. |
| CI | Read-only checkout, credential persistence disabled, timeout and current action major versions. New CI results must be checked after push. |
| Pages | Public website previously verified; publishing limited to site/ and manual deployment. Updated action runtime versions. |
| Dependency maintenance | GitHub Actions Dependabot configuration added. |
| Evidence | Both local Debug builds and CTest suites passed. Full product requirements remain proposed and require implementation and per-target verification. |

Residuals: review diagram semantics with responsible architects, choose branch governance, evaluate dependencies and actual target hardware, implement independent broker and complete the verification plan. The starter's single-session monotonic request ID rule is a provisional local contract, not a production wire protocol.
