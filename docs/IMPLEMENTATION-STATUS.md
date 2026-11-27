# Implementation status — main branch

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

All four CI lanes passed on 65d47614 (desktop 46061895128, generated service 46061895108, COVESA 46061895124, native 46061895160). Desktop CI passed on multi-endpoint, dispatcher, profile manifest, session renewal, diagnostic store, profile lifecycle and migration/boot reconciliation revisions through e3de9ef1 (run 46063781031). Native core/provider CI also passed through 05dc4631 (run 46063258049). The four-profile native matrix, durable NVS snapshot and independent native worker require completed runs on their own revisions. See [Actions](https://github.com/susmithmrm/ZARC/actions).

## Current stabilization increments

- [Session and lifecycle stabilization](STABILIZATION-INCREMENT.md): desktop CI passed on b52b8f70.
- [UART simulator correction](UART-EMULATOR-CORRECTION.md): native CI passed on ef9d12ed (run 46061138833); physical driver unchanged.
- [Effect ownership](EFFECT-OWNERSHIP-INCREMENT.md): identity, deadline and independent provider release. Initial CI exposed an incomplete desktop caller migration, corrected in fd3c8cd3; desktop, generated gRPC and local COVESA CI passed on that revision.
- [Continuous runtime](CONTINUOUS-RUNTIME-INCREMENT.md): portable single-owner orchestration and separate non-ztest native simulator application, desktop CI passed on fd3c8cd3 and generated manifest revision 92190ba5; native continuous application CI passed on the recorded 65d47614 baseline. No physical target support implied.

- [Versioned update journal](UPDATE-JOURNAL-INCREMENT.md): desktop CI passed on 00d6136a; native NVS remount and MCUboot compile coverage passed on 65d47614; no physical boot execution.
- [Native PM and Zbus](NATIVE-PM-ZBUS-INCREMENT.md): simulator provider tests passed on 65d47614; physical PM and observer timing remain open.
- [Measurement and change impact](MEASUREMENT-CHANGE-IMPACT.md): streaming offline trace evaluators and change impact inventory; desktop CI passed on 65d47614; no physical measurements acquired.

## Common runtime realization increments

| Increment | Scope and evidence |
|---|---|
| [Multi-endpoint runtime](MULTI-RUNTIME-INCREMENT.md) | Up to four independent endpoints, local/remote ownership, logical suspend/resume; desktop and earlier native CI passed |
| [Shared runtime dispatcher](RUNTIME-WIRE-INCREMENT.md) | Scoped discovery, observations, typed admission and retained results; desktop CI passed |
| [Four simulator product profiles](PROFILE-RUNTIME-INCREMENT.md) | Manifest-selected edge, companion, zonal and integrated builds; profile lifecycle and IPC fixture; desktop validation passed, native matrix pending |
| [Explicit session renewal](SESSION-RENEWAL-INCREMENT.md) | Bounded result retention and no automatic replay; process saturation test passed desktop CI |
| [Durable diagnostic snapshot](DURABLE-DIAGNOSTICS-INCREMENT.md) | Versioned CRC snapshot and NVS readback; desktop codec test passed, native extension pending |
| [Config migration and boot recovery](CONFIG-BOOT-RECOVERY-INCREMENT.md) | Exact migration ticket and verified boot phase reconciliation; desktop CI passed, MCUboot compile/native extension pending |
| [Independent native worker](NATIVE-WORKER-INCREMENT.md) | Separate simulated device owner, immutable queue exchanges and later readback; latest CI pending |

## Remaining gates

All 111 IDs are represented in the verification register. Candidate paths exist for 104; this is not requirement closure. Complete acceptance vectors, product manifests and independent accepted evidence remain mandatory.

- Extend tested reference discovery and reconciliation to selected production transports and product schemas; integrate approved broker quotas, MCU authentication and identity delegation.
- Complete selected physical board and driver configurations, integrate physical UART/CAN/Ethernet and IPC transport owners with the continuous runtime, connect native PM, Zbus and durable journals to approved product services, and qualify signed boot, trusted metadata, watchdog, NVS and reset behavior. The new simulator compositions do not constitute physical ports.
- Integrate actual AAOS VHAL Binder/gRPC, OEM VSIDL/SDV, CommonAPI, independent Ethernet peer and hypervisor IPC with approved identities and configuration. Local COVESA and ZARC gRPC runs do not qualify these deployments.
- Run real HIL, physical feedback, timing/resource, reset, power interruption and second-vendor qualification; review safety and security analyses with responsible integrators.

The desktop controller operates only a benign simulated indicator. No physical vehicle commands or production credentials are included. Keep the branch in review until evidence is accepted; no automotive safety or production cybersecurity claim.
