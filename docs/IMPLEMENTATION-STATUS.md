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
| Target admission, task inventory and timing analysis | [Target admission](TARGET-ADMISSION-INCREMENT.md) |
| Standalone operation, cancellation and reserved capacity | [Autonomy](AUTONOMY-INCREMENT.md) |
| Discovery, observation reconciliation and result retention | [Observation increment](OBSERVATION-INCREMENT.md) |
| Executable generated gRPC service | [Generated service](GENERATED-SERVICE-INCREMENT.md) |
| Actual COVESA runtime reference | [SOME/IP runtime](SOMEIP-RUNTIME-INCREMENT.md) |
| Native provider emulator and spy tests | [Provider tests](NATIVE-PROVIDER-TESTS.md) |
| Build provenance and supplier admission | [Release engineering](RELEASE-ENGINEERING-INCREMENT.md) |
| Requirement-level trace and evidence gate | [Verification register](REQUIREMENT-TRACE.csv) and [HIL procedure](HIL-AND-EVIDENCE.md) |

Desktop CI and generated gRPC CI succeeded through b77e025a (runs 46056631426 and 46056640681). Native CI passed on the earlier b29607b4 revision (run 46056122910). Actual COVESA local middleware integration passed on 0356573d (run 46056390381). Expanded native provider fixtures and later provenance changes require their own completed CI results. See [Actions](https://github.com/susmithmrm/ZARC/actions).

## Current stabilization increments

- [Session and lifecycle stabilization](STABILIZATION-INCREMENT.md): desktop CI passed on b52b8f70.
- [UART simulator correction](UART-EMULATOR-CORRECTION.md): native CI on patched simulator pending.
- [Effect ownership](EFFECT-OWNERSHIP-INCREMENT.md): identity, deadline and independent provider release. Initial CI exposed an incomplete desktop caller migration, corrected in the following revision; rerun required.
- [Continuous runtime](CONTINUOUS-RUNTIME-INCREMENT.md): portable single-owner orchestration and separate non-ztest native simulator application, tests committed. No physical target support implied.

## Remaining gates

All 111 IDs are represented in the verification register. Candidate paths exist for 98; this is not requirement closure. Complete acceptance vectors, product manifests and independent accepted evidence remain mandatory.

- Extend tested reference discovery and reconciliation to selected production transports and product schemas; integrate approved broker quotas, MCU authentication and identity delegation.
- Compile selected native board/driver combinations, integrate board and product manifests, implement product-side provider owners, persistent update journal and boot reconciliation; qualify UART/CAN/IPC, watchdog, NVS, signed boot and reset behavior.
- Integrate actual AAOS VHAL Binder/gRPC, OEM VSIDL/SDV, CommonAPI, independent Ethernet peer and hypervisor IPC with approved identities and configuration. Local COVESA and ZARC gRPC runs do not qualify these deployments.
- Run real HIL, physical feedback, timing/resource, reset, power interruption and second-vendor qualification; review safety and security analyses with responsible integrators.

The desktop controller operates only a benign simulated indicator. No physical vehicle commands or production credentials are included. Keep the branch in review until evidence is accepted; no automotive safety or production cybersecurity claim.
