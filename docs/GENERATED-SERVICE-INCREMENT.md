# Executable generated semantic service

The optional generated-service lane compiles the repository's protobuf contract, runs an actual gRPC server with a single worker and mutual TLS, and connects it to the trusted host gateway and independent desktop controller. A separate generated client reads the benign indicator and submits authorized intent. It checks denied and unknown certificates, accepted request identity and later feedback-confirmed observation. The server identifies clients by the verified certificate bytes exposed by gRPC, using the same fingerprint and capability policy as the JSON gateway.

The candidate dependency pins are in [host requirements](../host/requirements-grpc.txt). This is a real executable gRPC binding of ZARC's own schema. It is not the AOSP VehicleServer protocol, a VSIDL-generated Android SDV service, or a released deployment. Choose one gateway listener per backend owner. The MCU connection, selected external runtime and credential lifecycle still require approved integration and evidence. The workflow must pass before claiming a tested binding.

References: [gRPC server-side authentication](https://grpc.github.io/grpc/core/md_doc_server_side_auth.html), [AOSP VHAL reference](https://source.android.com/docs/automotive/vhal/reference-implementation).

Trace candidates: CON-001, HOST-005 and ASR-004/005. External stack interoperability and product release remain open.
