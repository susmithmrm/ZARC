# COVESA runtime fixture

This candidate uses the actual COVESA vsomeip3 runtime with separate service and client processes. The service is a read-only adapter: it translates a versioned SOME/IP method payload into an authenticated generated gRPC request to the existing trusted host gateway, and maps the observed result back to explicit big-endian fields. The COVESA adapter never opens or owns the controller connection. Its certificate grants only the approved fixture observation scope. Each call has a deadline; concurrent callbacks receive BUSY. No unauthenticated physical or desired-state command is offered.

The optional workflow builds COVESA 3.5.11, records the resolved revision, generates the semantic RPC bindings and runs the independent peer against the real middleware, gateway and desktop controller. The configuration is deliberately local routing on loopback; it does not verify Ethernet packets, multicast service discovery, external client authorization, CommonAPI-generated interface or an OEM network. The IDs are lab values, not product allocations. Actual remote interoperability and deployment evidence remain required.

Reference: [COVESA upstream](https://github.com/COVESA/vsomeip), [runtime guide](https://github.com/COVESA/vsomeip/blob/master/documentation/vsomeipUserGuide.md).

Trace candidates: HOST-006, NET-001, CON-001/003/004. Until CI passes, the binding is an unverified candidate. No requirement is closed.
