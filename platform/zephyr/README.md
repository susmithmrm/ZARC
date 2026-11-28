# Native Zephyr reference and provider fixtures

Development target: Zephyr v4.2.0 native simulator. Physical target support requires independent admission. Implementation and verification are the next phase.

From an initialized workspace, apply the documented simulator-only UART correction to a clean pinned Zephyr tree, then run all fixtures:

```sh
python /path/to/ZARC/tools/patch_uart_emulator.py /path/to/zephyr
west twister -T /path/to/ZARC/platform/zephyr -p native_sim/native/64 --inline-logs
```

The correction is only for the pinned emulator; see [ownership correction](../../docs/UART-EMULATOR-CORRECTION.md). Do not apply it to a physical BSP. The workflow archives the actual patch and upstream revision.

The non-ztest application in [runtime/](runtime) selects one of four candidate manifests (edge, companion, zonal, integrated) and runs the common runtime with a simulated device worker. For the default edge profile:

```sh
west build -b native_sim/native/64 /path/to/ZARC/platform/zephyr/runtime
west build -t run
```

For the zonal simulator, use a clean build and add `-- -DCONFIG_ZARC_REFERENCE_PROFILE=zonal`. The integrated variant additionally needs `CONFIG_IPC_SERVICE=y`. Twister selects each variant automatically. See [native execution design](../../docs/architecture/03-Native-RTOS-Software-Design.md) and [product line](../../docs/architecture/04-Product-Line-Hardware-Integration.md).

The provider fixtures exercise native UART/CAN/IPC/watchdog/NVS/Zbus/runtime-PM APIs with emulated devices and compile the MCUboot adapter. They do not establish physical bus, power, bootloader execution, timing, flash endurance, actual board support or automotive qualification. See [verification plan](../../docs/architecture/09-Verification-Engineering.md) and [implementation plan](../../docs/IMPLEMENTATION-STATUS.md).
