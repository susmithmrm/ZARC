# Native maintenance and supervision bindings

Candidate Zephyr v4.2.0 adapters. Call watchdog arm only from the approved hardware owner after reviewing reset scope and device capability; use the sole health supervisor to evaluate required progress before every feed. No periodic unconditional feed. The native watchdog configuration uses SoC reset and is unsuitable unless that reaction is explicitly allocated. A setup error may require device reset before retry.

The NVS binding reads and writes the explicit two-slot configuration byte records with read-back comparison. The target integrator mounts a dedicated NVS partition, selects a key and sector geometry, serializes access in a maintenance worker, and validates write endurance and power interruption on hardware. An absent record is returned as an error, not silently populated. NVS integrity and the inner CRC do not authenticate configuration. Authorization and migration are external.

Sources are selected via the native configuration; no general RTOS facade. These are actual API bindings but no physical watchdog or flash measurement has been performed. See the [native fixture](../README.md).
