# Continuous benign reference runtime

The new portable runtime joins the existing controller, observation stream, composition, periodic release accounting, health supervision and diagnostic journal under one state owner. It blocks local admission until a fresh valid provider observation and mandatory services are ready. Missing or invalid periodic samples mark the service unavailable and record a bounded diagnostic on the health transition. The provider must explicitly release its effect ownership; stop rejects unresolved work. No physical fail reaction is inferred.

The separate native Zephyr application builds without ztest. It has evolved into four manifest-selected simulator profiles with one to four independently supervised benign endpoints, shared scoped frame dispatch, explicit logical lifecycle and a separate device-owner worker. See [profile runtime](PROFILE-RUNTIME-INCREMENT.md), [multi-endpoint runtime](MULTI-RUNTIME-INCREMENT.md), [runtime dispatcher](RUNTIME-WIRE-INCREMENT.md) and [native worker](NATIVE-WORKER-INCREMENT.md). The generated configuration includes endpoint IDs, timing, stack and priority allocations. Normal mode does not issue an effect; smoke mode exercises benign intent, later readback, disconnect, suspend/resume and stop. The full manifest digest is printed; boot identity remains fixed only for isolated simulation. No measured WCET is claimed.

```sh
west build -b native_sim/native/64 /path/to/ZARC/platform/zephyr/runtime
west build -t run
```

Outstanding: physical provider owners and external transports, measured execution timing, complete generated product policy and resources, actual wake/suspend, durable maintenance-worker integration, trusted boot identity, hardware and external stack evidence. No production physical control or safety qualification claimed.
