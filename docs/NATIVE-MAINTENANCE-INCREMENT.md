# Native simulator durable startup and shutdown

Each of the four native simulator profiles mounts the dedicated simulator NVS partition before the runtime starts. A per-profile boot counter is committed and read back before admission, and restored diagnostics are checked against the previous boot epoch. The independent device owner receives that boot identity and rejects effects belonging to a different boot. No flash write occurs in the active real-time cycle.

After the runtime and device worker stop, the maintenance owner stores the bounded journal, remounts the partition and compares the complete serialized readback. The existing simulator smoke scenarios now exercise this path. Startup refuses malformed storage and does not automatically erase or replace damaged records. The boot counter is a simulator persistence exercise, not a secure monotonic counter: flash cloning, corruption, deletion or reinitialization can reuse IDs. No physical boot or storage endurance claim is made. Continuous asynchronous fault checkpoints, trusted boot identity and recovery under power loss remain product-specific work.

The simulator storage area must not contain production or customer data. The profile descriptor selects NVS and records these limitations; physical qualification remains open.
