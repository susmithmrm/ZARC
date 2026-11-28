# Z04 — Product line, silicon and deployment units

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Native integration
Use maintained/owned Zephyr ports, approved vendor SDK, pinctrl/devicetree, Kconfig, west, CMake and sysbuild. Thin adapters for domain semantics or genuine vendor gaps. Manifest resolves ISA, ABI, board, stepping, core/mode, memory, toolchain, features and artifact identities. Record errata, electrical limits, licensing, hardware protection and provider ownership. Unsupported targets are not native support.
## Deployment units

| Model | Scope |
|---|---|
| A whole image | Resource-constrained MCU; independently versioned components statically linked |
| B multi-image | Supported MCU/SoC domains with allocated boot and recovery |
| C isolated module | Only admitted isolation, loader, memory and recovery capability |
| Data unit | Configuration, calibration or selected model with schema and activation policy |
| Vendor/service replacement | Immutable or vendor-owned boot/firmware/hardware with explicit policy |

PL-025 inventory includes every releasable artifact; no universal hot updates. PL-024 reconciles intended/configured/built/deployed/observed identity. Unknown and discrepant fields remain explicit.
## Portfolio
Common benign source on at least two independent physical MCU vendors; automotive MCU, general-purpose MCU and eligible heterogeneous SoC. Arm/RISC-V and DSP/NPU depend on port/driver eligibility. Evidence per exact profile, not brand-level support. Native peripherals, sensing, actuation, 12/48 V supervision, factory provisioning, calibration and replacement require specific electrical and safety allocations. No real vehicle actuation in lab reference.

## Target admission
Resolve exact pinmux, clocks, electrical assumptions, interrupts, native device readiness and assigned I/O ownership before activation. Separate model, simulator, compiled target, physically exercised target and accepted product configuration. Use configuration-specific identity and evidence, not vendor name or ISA as proof. Hardware integration package includes supported compiler/ABI, native driver dependencies, errata, constraints and independently reviewable conformance fixtures.

## Product configuration and compatibility
PL-001–014, PL-018/019/023–025 allocate native configuration, exact hardware and software identity, protection, ABI and deployment boundaries. Compare intended, configured, built, deployed and observed facts with provenance; no second authoritative hardware description. Inventory each artifact and identify its activation, recovery or replacement policy. Maintain an evidence-level support matrix per exact configuration, not a claim of support by vendor name.
## Device and product integration
ADOPT-001–010, ADOPT-074/077 allocate portfolio coverage, typed sensors, independently observed actuator feedback, electrical review, calibration, peripherals, power management, factory identity, silicon lifecycle, domain packs and authorized service lifecycle. Maintain calibration separately from board wiring and product policy, and report measured values with units, time, quality and identity. Manufacturing, service and retirement obligations are allocated to the accountable product owner. The physical portfolio covers automotive MCU, resource-constrained MCU and eligible heterogeneous SoC; two independent MCU vendors and the separate distributed/zonal topology proof remain required.
