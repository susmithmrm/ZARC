# Implementation status — development branch

The architecture baseline has 111 proposed requirements. The following increments extend the original desktop prototype; they are not accepted product evidence.

| Component | Implementation and evidence |
|---|---|
| Portable indicator domain | Existing domain and contract tests |
| Composition and lifecycle | [Composition increment](COMPOSITION-INCREMENT.md) |
| Health, diagnostics, configuration model | [Supervision increment](SUPERVISION-INCREMENT.md) |
| Sessions, frame codec, result retention, cache | [Connect increment](CONNECT-INCREMENT.md) |
| Integrated controller and endpoint instances | [Controller increment](CONTROLLER-INCREMENT.md) |
| Independent controller and broker processes | [Loopback increment](LOOPBACK-INCREMENT.md) |
| Guest readiness and firmware update decision models | [Coordination increment](COORDINATION-INCREMENT.md) |
| Native Zephyr candidate fixture and CI | [Native increment](NATIVE-INCREMENT.md) |
| UART, CAN and IPC API bindings | [Transport adapters](../platform/zephyr/adapters/TRANSPORT.md) |
| Native watchdog, NVS and MCUboot bindings | [Native adapters](../platform/zephyr/adapters/README.md) |
| TLS guest identity and authorization gateway | [Host security increment](HOST-SECURITY-INCREMENT.md) |
| AAOS and typed SDV/SOME-IP payload mapping | [Host mapping increment](HOST-MAPPING-INCREMENT.md) |
| Requirement-level trace and evidence gate | [Verification register](REQUIREMENT-TRACE.csv) and [HIL procedure](HIL-AND-EVIDENCE.md) |

Desktop CI success observed through host mapping commit 45b56f69 (run 46048240779). Later revisions require their own CI results. The new native CI workflow is running; no completed native build was observed at the time of this record. The optional physical adapters are not all selected by the simulator configuration. See [Actions](https://github.com/susmithmrm/ZARC/actions).

## Remaining gates

All 111 IDs are represented in the verification register. Candidate paths exist for 67; this is not requirement closure. Complete acceptance vectors, product manifests and independent accepted evidence remain mandatory.

- Complete product schema and discovery, broker quotas and event reconciliation; select and verify MCU authentication and identity delegation.
- Compile selected native board/driver combinations, integrate board and product manifests, implement product-side provider owners, persistent update journal and boot reconciliation; qualify UART/CAN/IPC, watchdog, NVS, signed boot and reset behavior.
- Integrate actual AAOS VHAL Binder/gRPC, VSIDL/SDV, CommonAPI/vsomeip peer stack and hypervisor IPC with approved identities and configuration.
- Run real HIL, physical feedback, timing/resource, reset, power interruption and second-vendor qualification; review safety and security analyses with responsible integrators.

The desktop controller operates only a benign simulated indicator. No physical vehicle commands or production credentials are included. Keep the branch in review until evidence is accepted; no automotive safety or production cybersecurity claim.
