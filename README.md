# ZARC desktop development

Portable C17 first slice using a simulated indicator. Open ZARC.code-workspace in VS Code. CMake 3.21+ and a C17 compiler are required.
## Quick start

### Windows

```powershell
cmake --preset desktop-windows
cmake --build --preset desktop-windows
ctest --preset desktop-windows
```

### Linux

```bash
cmake --preset desktop-linux
cmake --build --preset desktop-linux
ctest --preset desktop-linux
```
Use the desktop-windows or desktop-linux CMake preset, and use a separate build directory for each operating system. The tests cover admission, feedback confirmation, timeout, duplicate handling and independent instances. The starter retains only one request per instance, refuses older IDs and assumes a single simulated principal/session. It is a prototype, not a full contract implementation.
See docs for the architecture and realization allocation. Future increments include independent broker processes, service registry, session contracts, diagnostics, persistence, target adapters and target evidence. No real automotive hardware is operated by this starter.

## Documentation

[Architecture index](docs/architecture/00-README.md) · [Diagram atlas](docs/architecture/diagrams/README.md) · [Project site](https://susmithmrm.github.io/ZARC/) · [Review findings](docs/REPOSITORY-AUDIT.md)

## License

ZARC is licensed under the [MIT License](LICENSE). Third-party dependencies retain their respective licenses.
