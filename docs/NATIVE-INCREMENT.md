# Native Zephyr increment

A Zephyr-native ztest project is in [platform/zephyr](../platform/zephyr/README.md). It compiles the portable source set and uses native message queues, a separate device worker and monotonic ticks. One controller owner serializes state. The bounded effect queue is reserved by its only producer before admission; the virtual device worker publishes observations through a separate bounded queue. This verifies a candidate execution mapping only when the Twister test passes.

Trace candidates: RT-001, CON-007/012/016. No requirement is closed. The fixture was not compiled or executed in the authoring environment. Selected Zephyr version, target board, real device ownership, scheduling analysis, physical feedback, native transport, boot, storage and HIL remain open.
