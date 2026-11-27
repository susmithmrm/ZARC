# Read-only update status

The common runtime supports an optional locally published, versioned update-state snapshot. A trusted maintenance owner must supply verified, reconciled metadata and a strictly increasing publication revision. Struct validation is shared with the journal codec. Unverified, malformed or stale publication invalidates status; suspend invalidates it as well. The allocated freshness window is explicit and disabled by default. This is not an image verification or update execution API.

Operation 9 uses an empty request and an independent controller-wide read scope. Completed response: status (u8), schema (u8=1), phase (u8), publication revision, active image ID/version, candidate image ID/version, sample tick (six little-endian u64 values). The frame supplies boot and session. TLS JSON and generated gRPC require a distinct update-read grant, which defaults to false. With no approved provider, they return UNAVAILABLE, not fabricated active firmware metadata.

The C contract test supplies a benign decision-model snapshot; generated gRPC integration verifies explicit denial and unavailable state. Real verified running-image metadata, durable publisher integration, authenticated MCU link and signed deployment remain outstanding. There is no remote firmware-write, boot or reset operation.
