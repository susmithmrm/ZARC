# Build provenance and supplier admission increment

Desktop CI collects the tracked source identity, per-file SHA-256 inventory, compiler metadata, build outputs and JUnit results. Native CI records its resolved Zephyr revision, active west manifest, generated configuration, devicetree, linker map and raw Twister logs. These are candidate build evidence, not an approved reproducible release or a complete dependency SBOM.

Run `python tools/build_evidence.py --build build --configuration desktop-lab --out build-evidence.json` from the checkout after building. The collector rejects modified tracked sources. The release evidence gate requires the actual configuration manifest artifact and verifies its digest as well as test artifacts. Reviewers must independently accept the configuration and evidence.

Supplier submissions can be checked with `python tools/supplier_admission.py submission.json`. Required fields cover identity, immutable revision, maintenance and change policy, license, API contract and lifecycle declarations, budget, limitations, evidence and dependency review references. The lifecycle declaration contains exact supported/deprecated versions and caller-declared compatible-upgrade pairs; it does not infer semantic compatibility. Only CANDIDATE is admitted by this structural checker. Supported and qualified status need accepted product and supplier evidence; no partner or approval is invented.

Outstanding: dependency-complete SBOM and license acceptance, pinned production compiler and board configuration, release signing, supplier agreements, independent review and release board decision.
