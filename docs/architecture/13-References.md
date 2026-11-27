# Z13 Authoritative source register

Research checked 27 November 2026. Public sources establish upstream capabilities and published directions, not ZARC implementation, adoption, measurements or qualification. Pin exact releases and licensed standard texts when baselining requirements. References marked as candidates do not establish project maturity or fitness.

- <a id="s01"></a> **S01** — Zephyr project documentation and release process: native kernel/device/build facilities and lifecycle. https://docs.zephyrproject.org/latest/ ; https://docs.zephyrproject.org/latest/project/release_process.html
- <a id="s02"></a> **S02** — Zephyr Twister and pytest harness: test orchestration and multi-DUT support. https://docs.zephyrproject.org/latest/develop/twister/harness/pytest.html
- <a id="s03"></a> **S03** — Zephyr safety overview: limited-scope safety initiative. https://docs.zephyrproject.org/latest/safety/index.html ; https://github.com/zephyrproject-rtos/zephyr/blob/main/doc/safety/safety_overview.rst
- <a id="s04"></a> **S04** — Zephyr memory domain APIs and virtual memory: hardware-dependent access constraints. https://docs.zephyrproject.org/latest/doxygen/html/group__mem__domain__apis.html ; https://docs.zephyrproject.org/latest/kernel/memory_management/virtual_memory.html
- <a id="s05"></a> **S05** — MCUboot design: supported boot/image test and revert modes. https://docs.mcuboot.com/design.html
- <a id="s06"></a> **S06** — Zephyr sysbuild images: multi-image orchestration. https://docs.zephyrproject.org/latest/build/sysbuild/images.html
- <a id="s07"></a> **S07** — Zephyr IPC service: instance and endpoint abstraction with backend-specific features. https://docs.zephyrproject.org/latest/services/ipc/ipc_service/ipc_service.html
- <a id="s08"></a> **S08** — AOSP automotive power management: VMCU/VHAL and Android power lifecycle. https://source.android.com/docs/automotive/power/power
- <a id="s09"></a> **S09** — AOSP automotive virtualization architecture: VHAL client/server reference approach. https://source.android.com/docs/automotive/virtualization/architecture
- <a id="s10"></a> **S10** — AOSP AAOS SDV architecture: isolated VMs, VSIDL, SOME/IP and orchestration. https://source.android.com/docs/automotive/sdv/sdv-system-architecture
- <a id="s11"></a> **S11** — AOSP SDV Core integration guide: host, direct and virtualized models. https://source.android.com/docs/automotive/sdv/workstreams/core/integration-guide
- <a id="s12"></a> **S12** — AOSP SDV gateway on IVI: approved service integration approaches. https://source.android.com/docs/automotive/sdv/sdv-gateway-on-ivi
- <a id="s13"></a> **S13** — COVESA vsomeip repository: SOME/IP implementation. https://github.com/COVESA/vsomeip
- <a id="s14"></a> **S14** — COVESA CommonAPI SOME/IP runtime and guide: binding and dependencies. https://github.com/COVESA/capicxx-someip-runtime ; https://github.com/COVESA/capicxx-someip-tools/blob/master/docx/CommonAPISomeIPCppUserGuide
- <a id="s15"></a> **S15** — Uptane standard: Primary/Secondary update roles and verification. https://uptane.org/docs/latest/standard/uptane-standard
- <a id="s16"></a> **S16** — Eclipse OpenBSW project: embedded automotive C++ stack, reuse candidate. https://projects.eclipse.org/projects/automotive.openbsw
- <a id="s17"></a> **S17** — Eclipse LIBERSOM project: incubating modular embedded SOME/IP stack, evaluate compatibility and footprint. https://projects.eclipse.org/projects/automotive.libersom
- <a id="s18"></a> **S18** — Eclipse OpenSOVD: open SOVD integration with S-CORE, host diagnostics candidate. https://projects.eclipse.org/projects/automotive.opensovd
- <a id="s19"></a> **S19** — COVESA Vehicle Signal Specification: data semantics candidate at host/tooling boundary. https://github.com/COVESA/vehicle_signal_specification
- <a id="s20"></a> **S20** — BMW Group public announcement: centralized computing and zonal architecture. https://www.press.bmwgroup.com/global/article/detail/T0448372EN/four-superbrains-for-the-neue-klasse%3A-more-intelligent-more-efficient-more-powerful
- <a id="s21"></a> **S21** — ISO/SAE 21434:2021 overview: cybersecurity engineering. https://www.iso.org/standard/70918.html
- <a id="s22"></a> **S22** — ISO 24089:2023: software update engineering. https://www.iso.org/obp/ui/?_escaped_fragment_=iso%3Astd%3Aiso%3A24089%3Aed-1%3Av1%3Aen
- <a id="s23"></a> **S23** — ISO November 2026 update: draft ISO 26262 revisions. https://www.iso.org/files/live/sites/isoorg/files/news/magazine/ISOupdate/EN/2026/iso_update_202611_en.pdf
- <a id="s24"></a> **S24** — UK Vehicle Certification Agency: UN R155/R156 vehicle cybersecurity/software updating overview. https://www.vehicle-certification-agency.gov.uk/connected-and-automated-vehicles/cyber-security-and-software-updating/

- <a id="s25"></a> **S25** — Zephyr bus: publishers perform dispatch; synchronous, asynchronous and message subscriber observer semantics. https://docs.zephyrproject.org/latest/services/zbus/index.html
- <a id="s26"></a> **S26** — Zephyr scheduling: priority ordering, cooperative/preemptive behavior and deadline tie-breaking. https://docs.zephyrproject.org/latest/kernel/services/scheduling/index.html
- <a id="s27"></a> **S27** — Zephyr workqueues: FIFO handler behavior, pending work lifetime, queued-item semantics and blocking. https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html
- <a id="s28"></a> **S28** — Zephyr system/device PM API and runtime usage references: feature and driver specific. https://docs.zephyrproject.org/latest/doxygen/html/group__subsys__pm__device.html ; https://docs.zephyrproject.org/latest/samples/subsys/pm/device_pm/README.html
- <a id="s29"></a> **S29** — Zephyr UART overview: alternative UART APIs and exclusive callback caution. https://docs.zephyrproject.org/latest/hardware/peripherals/uart.html
