# Z04 Product line and hardware integration
## Composition model (D08)
A supported build is the immutable composition of ZARC release, pinned Zephyr/toolchain, target board/SoC revision, BSP and vendor modules, product policy, enabled service/transport set, interface versions, security/boot/update profile and verification configuration. A board is wiring, target is composed configuration, product is policy. Do not infer vehicle function from board name. Source portability is not supported-target evidence.

| Layer | Normative owner | Deliverable |
|---|---|---|
| Zephyr and upstream modules | Maintainers/vendors | Pinned revisions, license and supported API |
| SoC/board BSP | Semiconductor/board supplier | DTS, pinctrl, clocks, reset, memory, drivers, errata |
| Target integration | Integrating Tier 1/vendor | Device binding, target manifest, approved resources |
| Product configuration | OEM/Tier 1 | Capability IDs, policy, service variants, calibration |
| ZARC common services | Platform maintainers | Public contracts and test suite |
| Evidence pack | Each responsible supplier | Scoped results, assumptions and changes |

## Mandatory target descriptor fields
Target ID/revision; SoC/arch/board qualifier and silicon stepping; Zephyr/SDK/compiler revisions; selected profile and required services; physical resource ownership and mapping; DTS overlay/Kconfig and generated configuration hashes; flash/boot/update partitions; watchdog/reset/clock and power capabilities; selected link providers; available resource budgets; security provisioning requirements; isolation capabilities and limitations; physical and simulation test profiles; evidence references. Descriptor is validated at build time and generates compact capability tables; no general YAML/JSON interpreter in control path. Unknown required capability or incompatible resource assignment fails build/admission. Product variants can change approved policy without copying generic source. Never allow unrestricted runtime reconfiguration of allocated safety behavior.

## Repository layout and dependency rules
west.yml; app/{edge,companion,zonal,integrated}; modules/zarc/{core,services,connect}; platform/{board_support,target_adapters}; products/{reference,customer}; host/{broker,adapters}; interfaces/{schemas,mappings,fixtures}; config/{targets,profiles,budgets}; tests/{unit,contract,sim,hil}; assurance/{requirements,evidence}; docs/. Use west modules and standard Zephyr board/SoC/driver conventions; avoid duplicating upstream board definitions. CMake/Kconfig select variants and reject illegal combinations; devicetree describes hardware, product manifest declares logical policy. Sysbuild supports multi-image composition subject to version [S06]. Generate manifest from actual build and review differences. Code ownership checks reject generic-to-board/product dependencies, private-header coupling and unauthorized symbol overrides.

## Vendor porting contract
1. Agree product/profile requirements and target capabilities with OEM/Tier 1.
2. Validate SoC support, silicon stepping, errata, boot/reset/power and independent protection; select Zephyr revision and toolchain.
3. Supply native board/SoC configuration, native device drivers or upstream/backport plan; document third-party licenses.
4. Implement only required target adapters and mapping, no generic service edits for normal port.
5. Allocate IRQ/DMA, memory, partitions, clocks, watchdog, power and electrical limits; report unsupported capability explicitly.
6. Build and run target conformance, representative resource/timing measurements, simulation if suitable and physical HIL; compare against expected logical contract.
7. Submit evidence bundle: manifest, config hash, source commits, assumptions, measured budgets, limitations, defects, test data and supplier sign-off.
8. Approve supported profile only for tested configuration; re-evaluate changes and publish maintenance owner.
Potential hardware discovery: NXP S32 families, Infineon AURIX/TRAVEO, Renesas RH850, ST automotive families, TI automotive MCU and heterogeneous SoCs. These are candidate vendor engagements, not supported boards. Initial use requires published or vendor-provided support evidence. No generic portability claim for a processor family or all boards.

## Feature profile matrix
| Capability | E | C | Z | I |
|---|---|---|---|---|
| Core/local policy/health | Required | Required | Required | Required |
| Power domain coordination | Optional | Required | Optional | Allocation-dependent |
| Edge device services | Typical | Optional | Required | Allocation-dependent |
| Host broker | Optional | Required | Typical | Typical |
| UART | Optional | First candidate | Optional | Debug/optional |
| Ethernet/SOME-IP | Optional | Extension | Candidate | Host-mediated/optional |
| IPC | Optional | Optional | Optional | Required candidate |
| Diagnostics/update | Product-dependent | Product-dependent | Product-dependent | Product-dependent |
Security, verification and protection requirements cannot be silently disabled by profile. A support statement identifies board, feature set, limitations, test and release evidence.

## Configuration authority and ABI policy
Kconfig selects features; devicetree identifies physical device topology and bindings; the product descriptor carries vehicle/logical policy; a generated manifest records resolved values. If input representations conflict, configuration fails rather than applying undocumented precedence. Target adapters are linked in composition root; supplier code does not edit Core or replace arbitrary weak symbols. Native Zephyr APIs are source interfaces and may change across releases; ZARC does not promise cross-compiler binary ABI for arbitrary modules. Define versioned C-facing domain contracts where needed, stable semantic IDs and explicit ownership of all buffer lifetimes. Require upstream release migration review, driver errata and configuration compatibility for customers. Portable public interfaces must not expose raw Zephyr objects to host or unrelated suppliers as a cross-process ABI.

## Target readiness maturity levels
| Level | Meaning |
|---|---|
| Candidate | Declared hardware discovery; no support |
| Buildable | Pinned native build and documented source scope |
| Bench validated | Selected profile passes agreed physical/HIL and resource tests |
| Supported | Exact configuration admitted with maintenance owner and accepted evidence |
| Assessed scope | Separately defined independent safety/cyber evidence; not implied by support |
The target manifest must publish actual level and limitations. Do not make support contingent on unsupported vendor peripheral behavior; substitute an approved driver/provider or mark capability unavailable.
