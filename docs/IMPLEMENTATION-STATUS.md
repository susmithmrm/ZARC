# Starter implementation status

## Available now

| Component | Evidence |
|---|---|
| Portable C17 indicator domain | [Public header](../include/zarc/domain.h) and [implementation](../src/domain.c) |
| In-process demonstration | [Controller PC app](../apps/controller_pc.c) |
| Contract cases | [Test source](../tests/contract_tests.c) |
| Desktop build | [CMake](../CMakeLists.txt), [presets](../CMakePresets.json) |
| Continuous integration | [Workflow](../.github/workflows/ci.yml) and [runs](https://github.com/susmithmrm/ZARC/actions/workflows/ci.yml) |

The current API is zarc_init, zarc_request, zarc_feedback and zarc_tick. It supports one simulated domain instance per state object, one outstanding request, one retained result and increasing IDs in an assumed session. Authorization is represented by a caller-provided Boolean in the fixture. Deadline is exclusive. Feedback changes the observed state independently of request acceptance. The prototype does not implement an authenticated session or wire protocol.

## Pending

The service registry, independent broker, transport providers, persistence, diagnostics, update workflow and native Zephyr adapters remain planned. All 111 architecture requirements are proposed, with no accepted product evidence. Desktop tests do not establish hardware timing or qualification.

[Architecture baseline](architecture/00-README.md) · [Realization plan](architecture/realization/16-Implementation-Realization.md)
