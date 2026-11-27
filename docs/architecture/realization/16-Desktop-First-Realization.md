# ZARC realization specification

27 November 2026 • Desktop-first engineering edition • Companion to architecture 1.1

## Engineering decision

Build the controller's behavioral core once as a portable, bounded C17 library. Link that library into ordinary Windows and Linux processes first. Later link the same sources into the Zephyr firmware. Build the broker as a separate process so it can genuinely disappear, reset and fall behind. Windows and Linux here are development hosts. ZARC Host is the in-vehicle broker role. These meanings are independent.

This edition proposes C17 for the shared controller and CMake for portable builds. The first target is a single simulated indicator with independent feedback. These are reversible implementation choices. All design and product verification rows remain proposed. The parent architecture supplies the vehicle deployment and assurance obligations. This document governs the order of development and the boundary between shared logic and target-specific execution.

## Fresh review of the starting assumptions

| Starting assumption | Consequence | Revised design decision |
|---|---|---|
| A board must be picked at G0 | Service and lifecycle work is delayed by procurement | Start G0 with a host compiler, manifest and virtual clock. Select boards before the hardware phase. |
| All applications use native Zephyr primitives (RT-001) | Core cannot compile as an ordinary Windows library | Allocate RT-001 to the Zephyr execution adapter. Portable policy receives events and emits effects. |
| Native simulation covers desktop development | Toolchain and host support become accidental dependencies | Supply two ordinary desktop build jobs. Zephyr native simulation is an additional later lane. |
| One executable can simulate the whole system | Shared memory can hide disconnect, ownership and restart errors | Retain an in-process deterministic harness and a separate two-process integration fixture. |
| A passing desktop test establishes target behavior | Scheduler, peripherals, boot and timing can diverge | Replay the same vectors on the board and add electrical, timing and resource oracles. |
| The host sends a command and marks success | A command can appear successful after a dropped result or failed device | Record admission separately. Only an observation with matching domain, epoch and quality can confirm completion. |

## Work products and owners

The system architect allocates capabilities, local authority, dependencies and failure responses per profile. The software architect owns the public types, state machines, schema evolution and dependency graph. The embedded developer owns the shared reducer and effect adapters. The host developer owns client authorization, correlation, quotas and the state cache. The test lead owns fixtures and expected results independently of implementation. The integrator owns board bindings and measured budgets. The OEM and responsible assurance leads approve item-level safety allocations, operating modes and actual fault response.

For every change, start at a requirement ID in the allocation CSV. Record its profile and acceptance condition, choose an implementation unit below, write a failing or independently reviewed acceptance vector, implement the transition, attach results for each environment, and identify the hardware evidence still required. A feature is releasable only in the configuration for which its applicable evidence has been accepted.

## System design and deployment

The planned two-process desktop execution has five participants. The client issues authorized typed requests. The broker owns the only controller connection. The controller owns policy, service readiness and domain state. The virtual device accepts effects and emits observations after a configurable delay. The fixture driver owns virtual time and can interrupt communication or restart a participant. The fixture driver has no production interface. Keep observations independent of requests so tests can produce missing, delayed and contradictory feedback.

For standalone edge, omit the broker and link. Feed local observations directly into the same core. For a companion, place the core on an independent MCU and the broker on the SoC. For zonal, instantiate endpoint state and budgets separately, then test loss of one group alongside activity in another. For integrated RT, move the endpoint boundary to shared-memory IPC. The SoC vendor must establish reset, peripheral, memory, DMA and clock ownership for that configuration.

## Software design and repository contract

| Proposed unit | Inputs | Outputs and ownership | Allowed imports |
|---|---|---|---|
| contracts | Schema and immutable manifest | Fixed-width domain types, version constants, golden examples | Standard integer and size types |
| domain | Current controller instance, event, monotonic tick | New state, bounded effect list, explicit status | contracts |
| composition | Services and declared dependencies | Ordered init, rollback list and readiness bitmap | contracts, domain |
| connect | Bytes or message plus link context | Decoded envelopes, bounded outbound messages | contracts, codec |
| controller-app | Events from adapters | Serialized calls into one domain owner | domain, composition, connect and selected adapters |
| broker | Authenticated client and controller messages | Admission replies, result lookup, quality-aware snapshot | External contracts and desktop networking |
| platform/desktop | Simulated time, transport, device and persistent bytes | Deterministic event source or real socket callbacks | Desktop runtime |
| platform/zephyr | Native device events, threads, IRQ and timers | Same typed events and results | Zephyr, contracts |
| tests | Reviewed vectors and reference manifests | Per-case traces with configuration identity | Public entry points |

The future generic API is proposed as follows. `initialize(storage, config)` validates capacities and initializes exactly one instance. `dispatch(instance, event, tick, effects)` processes one event to completion. `snapshot(instance, output)` copies a consistent read-only observation. Teardown quiesces the instance and releases adapter-owned resources. Pass explicit lengths with buffers. The caller owns input bytes through dispatch return. A queued effect must own a copy or a uniquely transferred pool slot. No wall clock, sleep, socket, file handle, kernel object or vendor register enters the reducer. No universal task, mutex or driver wrapper is needed.

The dispatch loop handles one event at a time. A producer enqueues immutable events. The sole consumer checks identity, epoch, age, service readiness and range before changing state. Reserve all result/effect capacity before committing acceptance. Failure to reserve returns BUSY with no state mutation. Adapter callbacks enqueue completions asynchronously. Never call dispatch recursively from a device callback. Preserve ordering for required events. Coalesce only replaceable observations with a declared policy.

## State, errors and storage

Each controller has a boot identity, service graph, health map and per-domain observed value, quality and generation. Each peer link has a separate session identity, negotiated schema version, quotas and bounded result cache. Each accepted request has principal, session, operation, request ID, normalized arguments, deadline and outcome. Reusing the same key and arguments returns the retained result. Reusing the key with different arguments returns CONFLICT. The controller owns the pending operation after admission. Client timeout alone does not cancel it.

For the indicator fixture accept OFF or ON. Starting state is UNKNOWN until the virtual sensor reports. An accepted change queues one set-indicator effect. A matching feedback value before the exclusive deadline completes the operation. Observation generation advances when the observed value changes. A feedback fault reports FAULT. If no feedback arrives by the allocated deadline, record UNKNOWN_OUTCOME and mark quality according to product policy. On reconnect discard client-side cached validity, obtain a fresh snapshot and reconcile events after its generation. An event gap forces resynchronization. A controller reboot invalidates old boot-relative generations. Clock comparisons require a common clock or explicit uncertainty bounds.

Define one versioned configuration record with length, revision, integrity information and commit state. Simulate interruptions before staging, after staging and during activation. Startup chooses the last valid committed configuration or reports unavailable. Keep fault records bounded with occurrence count, service, epoch, generation, timestamp and quality. Store no credentials in trace fixtures. On actual targets map persistence to a reviewed flash scheme and test interruptions physically.

## Service implementation sequence

| Increment | Developer deliverable | Tester acceptance |
|---|---|---|
| Composition and lifecycle | Validate unique IDs and acyclic dependencies, start in order, undo acquired resources in reverse order | Duplicate/cycle refused before readiness. Inject failure at each init step. Verify independent service behavior. |
| Autonomous domain | Policy and feedback owner, bounded local input, immutable snapshot | Disconnect broker mid-transition. Local feedback still changes the authoritative snapshot. |
| Connect | Version negotiation, envelope bounds, status retention, event sequence | Truncated record, split/combined delivery, duplicate, conflicting ID, expired intent, reconnect and snapshot race |
| Host | Single owner connection, per-client authorization, queue and result lookup | Restart client then broker separately. No stale write replay. Non-owner client cannot change state. |
| Power and quiesce | Vehicle mode, controller, VM and service state machines with distinct owners | Missing guest readiness at deadline takes configured simulated fallback. Stop requires init/start. Resume rechecks feedback. |
| Health and diagnostics | Alive, deadline and logical checks plus bounded persistence | Advance virtual clock across deadline. Verify expected diagnostic and retention under repeated events. |
| Update model | Inactive candidate, compatibility check, trial, confirm and recovery states | Simulate reset at every transition including before confirmation. Require an approved compatible recovery state. |
| External adapters | Mock VHAL properties, SDV services, SOME/IP method/event pairs, IPC and remote endpoint groups | Compare typed fixtures, versions and errors. Record actual stack interop as a separate integration run. |

## Desktop build and test entry points

The existing CMake project provides `zarc_domain`, `zarc_controller_pc` and `zarc_contract_tests`. The future `zarc_broker_pc` target is pending. The domain library builds on both operating systems with only a C17 compiler and standard headers. Put Winsock startup, socket closing and OS error translation in the Windows adapter. Put POSIX descriptor and error handling in the Linux adapter. Define frames with explicit byte order and lengths. Never send an in-memory C structure. Treat partial sends, peer closure and receive boundaries explicitly. Use loopback and ephemeral ports for two-process tests. The loopback binding exercises desktop framing. Standard Ethernet SOME/IP conformance has its own peer suite.

Use the [README build presets](../../../README.md#quick-start). GitHub CI already builds Windows and Linux in Debug and Release; the current jobs exercise the indicator prototype only. Pin the tool versions in CI. Key portability cases include fixed-width counters, signedness, padding, byte order, UTF-8 fixture paths, monotonic resolution and wrap, socket shutdown, filesystem replacement and differing C runtime behavior. Compare decoded canonical event traces, excluding process IDs, paths and real-time timestamps.

## Deterministic acceptance scenario

Future integration fixture configuration grants principal operator access to indicator 1. A second principal has observation access. Virtual milliseconds start at zero. Start with boot B1, session S1 and good OFF feedback at generation 1.

| Step | Event | Expected observable result |
|---|---|---|
| 1 | Observer requests ON | Refusal. Zero device effects. Snapshot remains OFF. |
| 2 | Operator sends request 7 for ON | ACCEPTED. Exactly one device effect. Observed value remains OFF. |
| 3 | Repeat request 7 with identical arguments | Same retained status. Effect count stays one. |
| 4 | Reuse request 7 for OFF | CONFLICT. Pending request and observed value preserved. |
| 5 | Drop link before feedback, then deliver valid ON feedback | Local owner completes transition. Broker marks its cache unavailable. |
| 6 | Reconnect in S2 under boot B1 | Fresh snapshot says ON, generation 2. Broker discards S1 pending assumptions. |
| 7 | Restart controller as B2 and delay feedback | Quality UNKNOWN. Reject S1 work. Initial observed state comes from new feedback. |

Separate tests delay feedback past the exact deadline, saturate result capacity, inject a gap between subscribe and snapshot, restart the broker while the controller remains active, and initialize two controllers with different policy. Each case records setup, event list, expected status, effects, final state, actual trace, build ID and test environment. Define tie ordering for simultaneous events in each fixture. Test both orders for deadline/feedback and quiesce/intent races.

## Bringing up Zephyr and physical targets

First compile the unchanged behavioral sources with the chosen Zephyr toolchain. Bind the dispatcher to an owned thread and bounded native queues. Translate timer/driver observations into the existing contracts. Share golden vectors with the PC jobs. Next bind native UART with reviewed buffer ownership, map devicetree devices and power management, and repeat the indicator scenario with an independent measurement channel. Compare event order and results before tuning timing. Then measure stack, flash, RAM, CPU, IRQ and bus interference, deadline and wake behavior. Extend to a second vendor and record any core change the port required.

The PC update model covers decisions and interruption ordering. Boot trust, key provisioning, flash behavior, hardware watchdog, current draw, rail sequencing, reset domains and EMC require the appropriate board or vehicle evidence. AAOS power state direction, service discovery, actual SOME/IP serialization and hypervisor partitioning require the chosen external implementations. Link the evidence to the same configuration-specific requirement IDs.

## Delivery gates and handoffs

1. Desktop contracts. Architect signs the reference manifest, owners, time model, domain transitions and golden vectors. Developers can start without a Zephyr checkout.
2. Portable behavior. Developers deliver domain, composition and test runner. Test lead compares Windows and Linux results across nominal, saturation, restart and expiry fixtures.
3. Desktop system. Integrator delivers two-process controller and broker plus simulated device, update, diagnostic and VM clients. Test lead reviews independent failure traces and coverage against the allocation CSV.
4. Native bring-up. BSP supplier selects the board, pins dependencies, binds hardware and reruns conformance. RT engineer supplies target timing analysis and measurements.
5. Product integration. Tier 1 delivers chosen host/network stacks, second-vendor conformance, recovery results and bounded resource evidence. OEM closes vehicle allocations and accepts residual risk.

Each gate identifies build/config hashes, owners, unsupported capabilities, open defects and required evidence for the next gate. A passing behavior fixture closes only its test instance. The system-level requirement stays open until the responsible integrator accepts all applicable instances.
