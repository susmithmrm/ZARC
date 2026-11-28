# Z16 — Implementation realization plan

Implementation is the next phase. The single [Z12 register](../12-Requirements-Verification.csv) defines the required outcomes; the [development allocation](17-Development-Allocation.csv) identifies the responsible domain and acceptance.

## Design allocation
- Native Zephyr execution, configuration, device model and selected upstream services.
- Portable domain logic, typed component interfaces and static composition by default.
- Product-specific deployment units and explicit resource/authority ownership.
- Bounded observation and management, versioned interface and state compatibility.
- Optional transport, host, AI and engineering providers, selected by configuration.

## Implementation sequence
1. Inspect retained source and test code against Z12. Assign real paths only after review; remove duplication and preserve native facilities.
2. Implement or refine component descriptors, composition and resolved graph.
3. Establish observability, identity reconciliation and resource measurement.
4. Implement lifecycle, deployment-unit compatibility, update and recovery.
5. Integrate selected peers, physical targets and AI providers.
6. Run positive, negative, boundary, integration and configuration-specific acceptance tests; record evidence and independent review.

Do not declare any requirement accepted from design documents, source presence or a simulator alone. Physical hardware, vendor dependencies and independent qualification remain open.
