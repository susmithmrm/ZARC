# Continuous benign reference runtime

The new portable runtime joins the existing controller, observation stream, composition, periodic release accounting, health supervision and diagnostic journal under one state owner. It blocks local admission until a fresh valid provider observation and mandatory services are ready. Missing or invalid periodic samples mark the service unavailable and record a bounded diagnostic on the health transition. The provider must explicitly release its effect ownership; stop rejects unresolved work. No physical fail reaction is inferred.

A separate native Zephyr application (platform/zephyr/runtime) builds without ztest. It consumes the candidate edge manifest's generated order and digest, runs a continuous kernel sleep loop and keeps a simulated memory-backed indicator. The optional Twister smoke configuration explicitly exercises one benign effect, later independent simulated readback and stop. Normal mode does not automatically issue an effect. This is still a single-endpoint reference; the manifest does not yet generate every runtime parameter. Boot identity is fixed only for this isolated simulation.

```sh
west build -b native_sim/native/64 /path/to/ZARC/platform/zephyr/runtime
west build -t run
```

Outstanding: real provider worker, measured execution timing, remote dispatch and transport integration, complete generated product allocation, multiple profiles, approved wake/suspend, durable journal, boot identity, hardware and external stack evidence. No production physical control or safety qualification claimed.
