# Simulator UART ownership verification

For the pinned native-simulator workspace, the verification plan requires both receive buffers to be released before receive shutdown is complete. Use the repository's simulator-only correction procedure where required by the pinned emulator. Do not apply the correction to physical drivers or product BSPs.

Before verification, record the upstream version, exact simulator configuration and applied patch. Evaluate removal when an upstream version with equivalent behavior is admitted. Physical UART needs independent driver conformance, ownership, stress and timing evidence. No physical or upstream approval is implied.
