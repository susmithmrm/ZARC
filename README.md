# ZARC desktop development
Portable C17 first slice using a simulated indicator. Open ZARC.code-workspace in VS Code. CMake 3.20+ and a C17 compiler are required.
Run cmake -S . -B build, cmake --build build --config Debug, then ctest --test-dir build -C Debug --output-on-failure.
Use a separate build directory for each operating system. The tests cover admission, feedback confirmation, timeout, duplicate handling and independent instances. The starter retains only one request per instance and assumes a single simulated principal/session. It is a prototype, not a full contract implementation.
See docs for the architecture and realization allocation. Future increments include independent broker processes, service registry, session contracts, diagnostics, persistence, target adapters and target evidence. No real automotive hardware is operated by this starter.
