# Target composition and timing admission

The candidate descriptor in [config/targets](../config/targets/native-sim-edge.json) separates product policy from board wiring. The validator rejects unsupported mandatory services, missing dependencies, dependency cycles, duplicate resources, absent reset/boot/run/maintenance/shutdown owners, inconsistent features and scheduling inventories. Generated C tables have deterministic order and a SHA-256 descriptor digest. Resolved Kconfig can be checked against declared feature selection. The manifest deliberately permits only CANDIDATE maturity until a separately approved product admission process exists.

```sh
python tools/target_manifest.py config/targets/native-sim-edge.json --header build/generated/zarc_manifest.h
```

For a selected Zephyr build, add the resolved config with the Kconfig option to the validator. This checks selected feature bits, not the actual DTS ownership or image identity. The example budgets are hypothetical, not target measurements. The response-time analyzer implements constrained-deadline preemptive uniprocessor fixed-priority analysis with blocking, release jitter and higher-priority interference; ISR load must be included explicitly. It rejects unsupported assumptions, rather than reusing the result for SMP. Task WCET and interference bounds must be approved independently.

Trace candidates: SYS-002/006/007, PL-001/003/005/007/009/010/011, RT-002/007/009/019/022. Actual DTS/board evidence, supplier approval and immutable release metadata remain open. Tests exercise negative configuration and timing vectors; no requirement is closed.
