# Z07 — Central compute, host and fleet integration

> **ZARC 2035 | Design baseline candidate | 28 November 2026.** The single normative-candidate register is [Z12](12-Requirements-Verification.csv) (267 requirements). This design describes the intended architecture, not implemented or qualified functionality. See [implementation evidence](../IMPLEMENTATION-STATUS.md).

## Host boundary
Trusted broker exclusively owns physical link where selected. Guest, host, controller and service lifecycle independent. Scoped principals submit typed intents; local owner validates. Quotas and reserved resources protect independent functions. Restart clears stale validity and reconciles fresh snapshots; no replay of expired intent.
## Ecosystem
Selected AAOS VHAL, S-CORE/SDV, SOME/IP, gRPC, Linux/QNX or hypervisor adapters preserve semantics; host adapters replaceable independently. VHAL AP_POWER_STATE_REQ (VMCU→Android), AP_POWER_STATE_REPORT (Android→VMCU); pin branch and validate direction and states. Guest readiness is not physical readiness. Shared clock, boot, memory, device and DMA ownership need target evidence.
## Fleet and AI
Fleet coordinator/cloud are external optional integrations. Report actual installed identity, freshness, health, staged/active/recovered status and compatibility. Policy-defined rollout pause belongs to coordinator; activation only in eligible mode. AI enters authorized advisory boundary; heavy perception stays on suitable processor. No cloud requirement for local control.

## Retained engineering detail
Existing design detail is retained for compatibility and trace continuity. Where wording differs, Z12 and the current architecture above govern. Historical examples are not evidence of new functionality.

## Deployment and trust boundaries (D13)

For companion deployment, a trusted service VM or trusted host process exclusively owns the physical MCU link and ZARC Host broker. Hypervisor provides VM isolation, resource assignment, lifecycle and virtual devices; it does not execute physical control. AAOS IVI, SDV services, Linux/QNX and other approved clients communicate through appropriate host adapters. No guest direct UART or raw device access. Dedicated integrated real-time core can use native IPC through an approved bridge instead of UART; controller still owns physical state. VM layout, CPU/memory allocation, IOMMU, virtual network/firewall and device ownership are SoC-specific reviewed artifacts.

## Host component boundaries

| Module | Responsibility |
|---|---|
| Endpoint | Nonblocking link provider, session lifecycle, bounded queues |
| Broker | Sole physical endpoint owner, correlation, cancellation and request status |
| Policy | Authenticate/authorize approved clients by domain and role, quota enforcement |
| State store | Freshness, epoch/generation, immutable snapshot; unavailable on mismatch |
| VHAL adapter | AIDL/property semantics and power coordination |
| SDV adapter | Service bundle, VSIDL/data tunnel and service discovery as applicable |
| CommonAPI/SOME-IP | Typed proxy/stub and standard network binding |
| Diagnostic/update adapters | Approved vehicle diagnostics and fleet update interfaces |

Broker never blocks local RT; bounded priority queues, per-principal quotas and reserve core capacity; no lock held while waiting for link or client. One provider per controller instance unless reviewed redundancy/arbitration. On restart invalidate cache, establish session, retrieve capabilities and full snapshot; do not replay stale writes. Host cache is not authoritative feedback.

## Android Automotive integration (D14)

FACT: AOSP virtualized VHAL reference separates client and host server and uses gRPC/vsock [S09](13-References.md#s09). AOSP power management uses VMCU, VHAL and Android power service interactions [S08](13-References.md#s08). ZARC Host can implement vehicle backend in trusted environment; IVI VM uses approved VHAL client. Map supported property IDs, areas, access/change mode, units, range, rate and error/result to semantic schema. AOSP specifies that the VMCU uses AP_POWER_STATE_REQ to tell Android (via VHAL) to transition; Android writes AP_POWER_STATE_REPORT through VHAL towards the VMCU to report its state [S08](13-References.md#s08). The first field is state/enum; the second has state-dependent shutdown/postpone/sleep semantics. ZARC Host must map these exactly for its pinned branch; it must not reverse direction or represent generic device commands as these properties. Do not translate these directly into electrical rail commands. Verify exact property semantics against OEM branch and CTS/VTS obligations. Unsupported properties unavailable/not supported. Android CPMS owns Android lifecycle; ZARC local policy owns physical transitions.

FACT: AOSP SDV supports isolated VMs and direct deployments, and describes VSIDL/protobuf service contracts, inter-VM transport and SOME/IP integration [S10](13-References.md#s10), [S11](13-References.md#s11), [S12](13-References.md#s12). Map ZARC approved capabilities to SDV services/service bundles or gateway as appropriate; maintain version mapping and role checks. SDV orchestrator owns service readiness and desired vehicle modes; does not substitute for local physical protection.

## CommonAPI and SOME/IP integration

FACT: COVESA CommonAPI C++ SOME/IP runtime depends on vsomeip [S13](13-References.md#s13), [S14](13-References.md#s14). Generate or review .fidl/.fdepl from semantic interface, allocate service/instance/method/eventgroup IDs with OEM, configure routing/provider uniqueness and discovery according to profile. Method acceptance and observed state event separate. Do not assume software versions, multicast, gRPC or discovery imply delivery/authorization. Independent peers verify wire fixtures, versions, subscription, timeout and service disappearance. Linux/QNX adapter uses native OS capabilities and dependency support evidence; no automatic portability claim.

## Independent lifecycle coordination (D15)

Vehicle mode authority requests intent; physical controller validates local conditions; broker coordinates all required guest/service quiesce; hypervisor reports independent VM state. On wake, local permissible event, startup and feedback, host boot, session/descriptor negotiation, readiness, snapshot. On shutdown, vehicle mode intent and controller policy establish approved request/deadline; host aggregates readiness, reports actual status; controller rechecks conditions and owns physical outcome. Missing guest readiness follows vehicle-approved fallback rather than automatic arbitrary removal of power or indefinite hold. Updates can retain some domains while quiescing other services; eligibility and compatibility are product-specific. Guest restart and whole-SoC reset are distinct fault classes.

## Host topology and failure contract

The trusted environment can be native host, service VM or an explicitly authorized service domain; a hypervisor is not required in nonvirtualized integrations. The reference VHAL client/server transport is an example, not mandated for all implementations/branches. Establish the Android VHAL property contract separately from ZARC semantic API and vehicle-specific power mapping. Validate cancellation, shutdown postpone policy, AP suspend/resume, report direction and unavailable property behavior; no arbitrary modification of Android power state machine. A guest crash must not power cycle other domains without approved policy. A full SoC outage may remove broker, cache and client communications; continued operation of allocated independent controller functions is a design requirement that must be demonstrated. Multi-VM security needs hypervisor/SoC supplier isolation evidence, not simply VM naming. Separate adapter ownership, client authentication, traffic quotas and rolling compatibility. If redundant brokers are requested, define exclusive active ownership/fencing and recovery before enabling; default one broker only.
