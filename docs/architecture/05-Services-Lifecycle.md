# Z05 — Services, observability and lifecycle

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Contract and lifecycle
Each service has identity, release, dependencies, readiness, health, lifecycle, execution budget and access class. Freeze critical composition before operation. Reject missing dependencies, cycles and ambiguous ownership. Initialize in order; reverse rollback only acquired resources. REGISTERED → INITIALIZED → STARTING → READY → QUIESCING → SUSPENDED or STOPPED. Resume revalidates dependencies/feedback; stopped requires initialization. DEGRADED/FAILED carries cause. No host wait for allocated local functions.
## System Explorer contract
Authorized views cover every declared object or explicit limitation: service/component, CPU/memory/task progress, network, sensors and observed actuator feedback, power, storage, diagnostics, updates and selected model state. Identify owner, unit, timestamp, epoch, version, freshness, provenance, quality and unavailable reason. No unrestricted memory/debug or direct remote hardware access (SVC-028/029, ASR-029).
Collection is bounded; reserve critical capacity, count drops/coalescing, apply retention/privacy. Correlate benign events through graph and selected transports. Native tracing/logging are providers, not a separate scheduler. Minimal image may use local approved snapshot without network.
## Automotive services and AI
Select diagnostic event storage, function availability, mode/network management, persistent blocks, calibration, power-loss notification and supervision by product allocation. Use native storage and PM. Track schema transitions and interrupted-write recovery. No generic fault reaction or voltage sequence.
Optional advisory AI uses same descriptor, resource, inventory, health and lifecycle. Approved model/preprocessing identity, input quality, output provenance, monitoring and recovery; no direct physical authority. Disabled AI adds no exclusive runtime data/code.

## Service readiness and persistence
Distinguish registration, initialization, start, suspend, stop, resume and failure recovery. Roll back only successfully acquired resources. Cancellation cannot imply physical completion or free storage before provider quiescence. Persistent records define state schema, integrity, atomicity, endurance, migration, recovery and explicit invalidity. Diagnostics separate measured fault quality from event enable/storage policy; inhibited function availability is not healthy by default.
