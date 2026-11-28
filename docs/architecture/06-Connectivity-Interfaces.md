# Z06 — Open connectivity and engineering contracts

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Semantics
Typed intent, authorized scope, request identity, version, expiry and bounded retention. Acceptance is not completion; timeout may mean unknown outcome. Snapshot/event reconciliation detects gaps; producer boot epoch distinct from link session. Explicit freshness uncertainty, ordering, backpressure, error and compatibility. No raw C structure wire ABI. Bindings independently verified.
## Networks
Legacy CAN/CAN FD/LIN and selected DBC/LDF/authorized ARXML migration coexist with Ethernet, 10BASE-T1S, TSN and optional CAN XL/FlexRay. Separate protocol, PHY, clocks, buffers and physical proof. Selected ISO-TP/DoIP/UDS, SOME/IP, IPC, UART and J1939/ISOBUS profiles require independent peers. Time-sync loss explicit. No mandatory universal protocol or new stack.
## Engineering exchange
Selected VSS/SOVD, S-CORE, XCP/A2L, ASAM TDF/MDF/ODX or ReqIF/FMI as allocated and licensed. Publish pinned contract/format versions and supported fields. Trace correlation preserves identity, boot epoch and time uncertainty. Independent consumers demonstrate semantics. No unrestricted vehicle memory, credentials or actuator operations.
## Failure containment
Bound links and workers; isolate endpoints and reconcile after loss. Preserve local authority. Test representative legacy and future topologies using benign isolated fixtures.

## Provider contract
Specify endpoint identity, typed operations, units, producer epoch, sequencing, validity, expiry, bounds, and error/retry semantics. Each binding declares capabilities and independently verifies framing and recovery. Use bounded queues and allocated worker resources; under overload report gaps and stale state. Approved legacy and modern peer interfaces coexist; an unsupported mapping is a configuration error. Wire compatibility and application semantics are separate obligations.
