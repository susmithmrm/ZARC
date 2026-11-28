# Implementation status — main branch

The architecture baseline has 178 proposed requirements. The following increments extend the original desktop prototype; they are not accepted product evidence.

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
| UART, CAN, IPC and TCP socket API bindings (TCP simulator only) | [Transport adapters](../platform/zephyr/adapters/TRANSPORT.md) |
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

All four CI lanes passed on d06c27ca: desktop 46142706636, generated service 46142706667, COVESA 46142706652, and native Zephyr 46142706663. The native simulator run covers the new queue transfer/completion/cancellation fixture; the MCUboot configuration remains compile-only. These results do not establish physical target behavior or product qualification. See [Actions](https://github.com/susmithmrm/ZARC/actions).

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
| [Four simulator product profiles](PROFILE-RUNTIME-INCREMENT.md) | Manifest-selected edge, companion, zonal and integrated builds; profile lifecycle and IPC fixture; desktop and native simulator matrix passed |
| [Explicit session renewal](SESSION-RENEWAL-INCREMENT.md) | Bounded result retention and no automatic replay; process saturation test passed desktop CI |
| [Durable diagnostic snapshot](DURABLE-DIAGNOSTICS-INCREMENT.md) | Versioned CRC snapshot and NVS readback; desktop codec and native simulator remount tests passed |
| [Config migration and boot recovery](CONFIG-BOOT-RECOVERY-INCREMENT.md) | Exact migration ticket and verified boot phase reconciliation; desktop CI and native compile coverage passed; boot execution remains pending |
| [Independent native worker](NATIVE-WORKER-INCREMENT.md) | Separate simulated device owner, immutable queue exchanges and later readback; native simulator CI passed |

## Runtime and host service realization

All four engineering lanes passed on 06ef2c1d (desktop 46068427184, generated service 46068427186, COVESA 46068427234, native 46068427164). The website and all engineering lanes passed on 1bb24f16. Later increments require their own CI evidence; the following entries report implementation and observed results separately.

| Increment | Scope and evidence |
|---|---|
| [Runtime telemetry](RUNTIME-TELEMETRY-INCREMENT.md) | Scoped health and bounded counters; desktop and native CI passed on 06ef2c1d |
| [Native runtime providers](NATIVE-RUNTIME-PROVIDERS.md) | Zbus subscriber reconciliation and device PM integrated into all four simulator profiles; native CI passed on 06ef2c1d |
| [Shared desktop runtime](SHARED-DESKTOP-RUNTIME.md) | Desktop uses common dispatcher; generated health service; desktop, gRPC and COVESA CI passed on cefa324a |
| [Retained host outcome](HOST-RETAINED-OUTCOME.md) | Bounded authenticated principal/capability ownership; desktop, gRPC and COVESA CI passed on 496f5325 |
| [Native maintenance](NATIVE-MAINTENANCE-INCREMENT.md) | Simulator NVS startup boot counter and quiescent journal persistence/remount; desktop/gRPC passed on 17211517; native verification not yet confirmed |
| [Update status interface](UPDATE-STATUS-INTERFACE.md) | Scoped fresh local snapshot, explicit unavailable when no provider; generated gRPC passed on 210e93e6; other lanes pending |
| [SOME/IP health](SOMEIP-HEALTH-INCREMENT.md) | Executable read-only local method and independent peer regression committed on ab2b77f9; verification pending |

## Remaining gates

All 178 IDs are represented in the verification register. The current trace contains 140 PARTIAL_OPEN and 38 OPEN records across the 178 Z12 requirements; it does not mark any requirement accepted. Complete acceptance vectors, product manifests and independent accepted evidence remain mandatory.

- Extend tested reference discovery and reconciliation to selected production transports and product schemas; integrate approved broker quotas, MCU authentication and identity delegation. Guest-side retained outcomes are now exposed; complete multi-client event reconciliation, mode allocation and production update-status publisher remain open.
- Complete selected physical board and driver configurations, integrate physical UART/CAN/Ethernet and IPC transport owners with the continuous runtime, connect the tested simulator PM, Zbus and journal lifecycle paths to approved physical product services, and qualify signed boot, trusted metadata, watchdog, NVS and reset behavior. The new simulator compositions do not constitute physical ports.
- Integrate actual AAOS VHAL Binder/gRPC, OEM VSIDL/SDV, CommonAPI, independent Ethernet peer and hypervisor IPC with approved identities and configuration. Local COVESA and ZARC gRPC runs do not qualify these deployments.
- Run real HIL, physical feedback, timing/resource, reset, power interruption and second-vendor qualification; review safety and security analyses with responsible integrators.

The desktop controller operates only a benign simulated indicator. No physical vehicle commands or production credentials are included. Keep the branch in review until evidence is accepted; no automotive safety or production cybersecurity claim.

The unified baseline now contains 178 proposed requirements. New silicon-related obligations are open; refined acceptance criteria require renewed evidence review. Existing candidate paths are not accepted evidence.

## Candidate execution descriptor and baseline regression
The native simulator manifests now declare ISA, core, ABI, mode, core count, byte order, alignment and port ownership. Build-time validation rejects inconsistent declarations. Evidence gate regression derives the expected IDs from Z12 instead of a historical constant. Physical silicon eligibility and CI results remain to be verified.

## Numeric sensor reference contract
Optional portable C17 single-owner sample library adds typed scaled values, calibration identity, range, freshness, boot/sequence and quality handling. Desktop regression added. No physical provider integration or accepted HIL evidence; CI pending.

Physical candidate descriptor admission now checks declared address ranges, alignment, non-overlap, ownership, attributes, shared-memory coherency and external initialization dependencies. Synthetic negative fixtures are not actual linker or silicon evidence.

The optional sensor provider uses Zephyr sensor fetch/channel APIs and checked decimal conversion; native regression covers conversion boundaries and unavailable-device reporting. Physical acquisition and native CI results remain unverified.
