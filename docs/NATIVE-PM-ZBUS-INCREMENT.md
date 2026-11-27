# Native Zbus and runtime device PM

The dedicated observation channel uses native Zbus synchronous listener dispatch. The listener does only a fixed-size copy into a four-item kernel queue using K_NO_WAIT; no device access, network I/O or heap allocation. Publication or queue overflow latches a gap, and consumers must ignore queued observations until the owners quiesce, reset the channel queue and establish a new snapshot barrier. The fixture checks separate event copies, order and explicit saturation. The dedicated channel is a reference, not a dynamically configurable multichannel routing stack.

The native runtime-PM adapter checks device capability and exclusive usage, balances get/put and latches failures. It uses synchronous native API calls from the designated owner, not from ISR or a shared state lock. The simulator spy validates resume/suspend callback ordering and reference count. Actual wake latency, physical power, device driver capability, system PM and approved fallback remain target-specific. The tests do not qualify real hardware.

Trace candidates RT-018 and RT-021. Both remain open pending measured paths and configuration-specific acceptance. Provider Twister tests use simulated devices only.
