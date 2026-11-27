# Implementation status — development branch

The main architecture baseline has 111 proposed requirements. The following increments extend the original desktop prototype; they are not accepted product evidence.

| Component | Implementation and evidence |
|---|---|
| Portable indicator domain | Existing domain and contract tests |
| Composition and lifecycle | [Composition increment](COMPOSITION-INCREMENT.md) |
| Health, diagnostics, configuration model | [Supervision increment](SUPERVISION-INCREMENT.md) |
| Sessions, frame codec, result retention, cache | [Connect increment](CONNECT-INCREMENT.md) |
| Integrated controller and endpoint instances | [Controller increment](CONTROLLER-INCREMENT.md) |
| Independent controller and broker processes | [Loopback increment](LOOPBACK-INCREMENT.md) |
| Guest readiness and firmware update decision models | [Coordination increment](COORDINATION-INCREMENT.md) |
| Native Zephyr candidate fixture | [Native increment](NATIVE-INCREMENT.md) |

Desktop builds: see [CI](https://github.com/susmithmrm/ZARC/actions/workflows/ci.yml). Successful workflow runs have been observed through the loopback commit 3e1e2eb. Later revisions require their own CI results. Zephyr Twister has not been executed.

## Remaining gates

- Complete requirement-by-requirement traceability, reviewed product manifests and acceptance vectors; no automatic change to the proposed baseline.
- Production client authorization, trusted session and freshness mechanism, service schemas and discovery, broker policy, quotas, cancellation and full snapshot/event reconciliation.
- Native physical transport and device mappings, actual target watchdog, storage and boot/update integration, board ownership, timing, resource and reset evidence.
- Selected AAOS VHAL, SDV, SOME/IP and hypervisor/IPC interoperability, OEM-approved service definitions, second target/profile.
- Independent security and functional safety analyses, HIL and release qualification. No blanket safety or cybersecurity claim.

The desktop broker uses an explicitly lab-only principal; the storage and update components are reference models, not flash or cryptographic implementations. Physical controls are not included. Do not merge or deploy merely because desktop tests pass.
