# Capability discovery and event reconciliation

The controller's sole owner publishes bounded observation events on changes to value, quality, readiness or source metadata. A snapshot barrier captures the current observation and event cursor together. A reader asks for the next event under the boot identity; a stale boot, future cursor or overwritten history returns GAP. Reconciliation requires another barrier, not a stale valid claim. Eight events are retained, with no unbounded queue or pointer ownership across threads.

The desktop lab protocol implements capability enumeration, a subscription barrier, event polling and retained request status. Capability payloads contain ID and readiness, and do not make an unready endpoint available. The host loopback adapter uses discovery when connecting and provides status and event reconciliation operations. The private lab peer has no production authentication and its event poll is an owner-level operation, not a guest-facing subscription. Per-client subscription scope/rate and deployed transport evidence remain necessary.

Trace candidates: CON-001/005/008/009/015/016, SYS-006, HOST-002. Unit and process fixtures check barrier ordering, event loss, boot mismatch, request correlation and feedback independent of admission. No approved requirement closure is asserted.
