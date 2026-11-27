# Shared desktop runtime and health service

The private desktop loopback controller now uses the same continuous runtime and scoped dispatcher as the four native simulator profiles. The virtual device applies admitted effects after a bounded laboratory delay and reports independently sampled state; it retains effect ownership until completion/removal. Periodic observations may advance event cursors even while the observed value does not change. The process test consumes those events rather than assuming every event changes value.

The generated gRPC service and TLS JSON gateway expose the existing versioned GetHealth response through the same read scope as snapshots. The end-to-end generated client test checks successful health and denied unallocated capability. The legacy zero-length request-status query remains only as a lab adapter compatibility shim.

The desktop allocation uses a 50 ms cycle and 2 s health window for portable host scheduling. It is not measured target timing. The desktop socket remains loopback-only and unauthenticated downstream; no production MCU trust or vehicle actuator policy is provided.
