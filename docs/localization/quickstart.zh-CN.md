# ZARC 快速开始（简体中文）

翻译状态：草稿。尚未完成母语技术审校或合作伙伴验收。

ZARC 是以 Zephyr 为核心的汽车控制与集成框架，并包含可移植的 C17 核心。当前阶段为需求和架构设计，下一阶段开展软件实现与验证。桌面与 Zephyr 模拟器用于开发环境，不代表实体硬件支持或产品资格。请在 VS Code 中打开 `ZARC.code-workspace`。需要 CMake 3.21 或更高版本以及 C17 编译器。

有关受支持开发设置的英文原文，请参阅 [README 快速开始](../../README.md#quick-start)。术语请参阅[多语言术语表](GLOSSARY.md)，翻译和评审步骤请参阅[工作流程](WORKFLOW.md)。

## Windows

```powershell
cmake --preset desktop-windows
cmake --build --preset desktop-windows
ctest --preset desktop-windows
```

## Linux

```bash
cmake --preset desktop-linux
cmake --build --preset desktop-linux
ctest --preset desktop-linux
```

请使用与主机操作系统匹配的 preset。Windows 需要 MSVC，并且要能在命令行中使用 CMake；Linux 需要 Ninja，以及 GCC 或 Clang。请为不同操作系统使用各自独立的构建目录。

下一阶段计划见[实现计划](../IMPLEMENTATION-STATUS.md)。此快速开始运行桌面模拟；它不证明真实硬件运行、产品支持或独立安全资格。
