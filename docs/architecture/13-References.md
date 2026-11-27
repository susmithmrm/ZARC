# Z13 Authoritative source register
Research checked 27 November 2026. Public sources establish upstream capabilities and published directions, not ZARC implementation, adoption, measurements or qualification. Pin exact releases and licensed standard texts when baselining requirements. References marked as candidates do not establish project maturity or fitness.

- S01 Zephyr project documentation and release process: native kernel/device/build facilities and lifecycle. https://docs.zephyrproject.org/latest/ ; https://docs.zephyrproject.org/latest/project/release_process.html
- S02 Zephyr Twister and pytest harness: test orchestration and multi-DUT support. https://docs.zephyrproject.org/latest/develop/twister/harness/pytest.html
- S03 Zephyr safety overview: limited-scope safety initiative. https://docs.zephyrproject.org/latest/safety/index.html ; https://github.com/zephyrproject-rtos/zephyr/blob/main/doc/safety/safety_overview.rst
- S04 Zephyr memory domain APIs and virtual memory: hardware-dependent access constraints. https://docs.zephyrproject.org/latest/doxygen/html/group__mem__domain__apis.html ; https://docs.zephyrproject.org/latest/kernel/memory_management/virtual_memory.html
- S05 MCUboot design: supported boot/image test and revert modes. https://docs.mcuboot.com/design.html
- S06 Zephyr sysbuild images: multi-image orchestration. https://docs.zephyrproject.org/latest/build/sysbuild/images.html
- S07 Zephyr IPC service: instance and endpoint abstraction with backend-specific features. https://docs.zephyrproject.org/latest/services/ipc/ipc_service/ipc_service.html
- S08 AOSP automotive power management: VMCU/VHAL and Android power lifecycle. https://source.android.com/docs/automotive/power/power
- S09 AOSP automotive virtualization architecture: VHAL client/server reference approach. https://source.android.com/docs/automotive/virtualization/architecture
- S10 AOSP AAOS SDV architecture: isolated VMs, VSIDL, SOME/IP and orchestration. https://source.android.com/docs/automotive/sdv/sdv-system-architecture
- S11 AOSP SDV Core integration guide: host, direct and virtualized models. https://source.android.com/docs/automotive/sdv/workstreams/core/integration-guide
- S12 AOSP SDV gateway on IVI: approved service integration approaches. https://source.android.com/docs/automotive/sdv/sdv-gateway-on-ivi
- S13 COVESA vsomeip repository: SOME/IP implementation. https://github.com/COVESA/vsomeip
- S14 COVESA CommonAPI SOME/IP runtime and guide: binding and dependencies. https://github.com/COVESA/capicxx-someip-runtime ; https://github.com/COVESA/capicxx-someip-tools/blob/master/docx/CommonAPISomeIPCppUserGuide
- S15 Uptane standard: Primary/Secondary update roles and verification. https://uptane.org/docs/latest/standard/uptane-standard
- S16 Eclipse OpenBSW project: embedded automotive C++ stack, reuse candidate. https://projects.eclipse.org/projects/automotive.openbsw
- S17 Eclipse LIBERSOM project: incubating modular embedded SOME/IP stack, evaluate compatibility and footprint. https://projects.eclipse.org/projects/automotive.libersom
- S18 Eclipse OpenSOVD: open SOVD integration with S-CORE, host diagnostics candidate. https://projects.eclipse.org/projects/automotive.opensovd
- S19 COVESA Vehicle Signal Specification: data semantics candidate at host/tooling boundary. https://github.com/COVESA/vehicle_signal_specification
- S20 BMW Group public announcement: centralized computing and zonal architecture. https://www.press.bmwgroup.com/global/article/detail/T0448372EN/four-superbrains-for-the-neue-klasse%3A-more-intelligent-more-efficient-more-powerful
- S21 ISO/SAE 21434:2021 overview: cybersecurity engineering. https://www.iso.org/standard/70918.html
- S22 ISO 24089:2023: software update engineering. https://www.iso.org/obp/ui/?_escaped_fragment_=iso%3Astd%3Aiso%3A24089%3Aed-1%3Av1%3Aen
- S23 ISO November 2026 update: draft ISO 26262 revisions. https://www.iso.org/files/live/sites/isoorg/files/news/magazine/ISOupdate/EN/2026/iso_update_202611_en.pdf
- S24 UK Vehicle Certification Agency: UN R155/R156 vehicle cybersecurity/software updating overview. https://www.vehicle-certification-agency.gov.uk/connected-and-automated-vehicles/cyber-security-and-software-updating/

- S25 Zephyr bus: publishers perform dispatch; synchronous, asynchronous and message subscriber observer semantics. https://docs.zephyrproject.org/latest/services/zbus/index.html
- S26 Zephyr scheduling: priority ordering, cooperative/preemptive behavior and deadline tie-breaking. https://docs.zephyrproject.org/latest/kernel/services/scheduling/index.html
- S27 Zephyr workqueues: FIFO handler behavior, pending work lifetime, queued-item semantics and blocking. https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html
- S28 Zephyr system/device PM API and runtime usage references: feature and driver specific. https://docs.zephyrproject.org/latest/doxygen/html/group__subsys__pm__device.html ; https://docs.zephyrproject.org/latest/samples/subsys/pm/device_pm/README.html
- S29 Zephyr UART overview: alternative UART APIs and exclusive callback caution. https://docs.zephyrproject.org/latest/hardware/peripherals/uart.html
