# Read-only SOME/IP health service

The actual COVESA local-runtime reference now exposes method 3, GetHealth, backed by the generated gRPC service and common runtime. The service uses the same restricted broker certificate and serialized owner as snapshots; it does not accept client-provided authority. Request is a four-byte big-endian capability ID. A completed response is 43 bytes: status, capability (u32), failure flags (u32), readiness (u8), ownership flags (u8), monotonic sample tick, source sequence, boot and session (u64 each), all multi-byte fields big-endian. Other statuses return only the status byte.

The independent COVESA client exercises snapshot followed by health and checks readiness, identity and nonzero sample sequence. Both methods remain read-only and local; no production Ethernet discovery, wire-level conformance, guest identity delegation or external controller commands are claimed. The method ID is a laboratory allocation, not an OEM service assignment.
