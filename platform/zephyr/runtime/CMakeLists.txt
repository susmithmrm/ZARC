cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zarc_runtime_reference LANGUAGES C)
set(ZARC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT CONFIG_ZARC_REFERENCE_PROFILE MATCHES "^(edge|companion|zonal|integrated)$")
 message(FATAL_ERROR "Unknown reference profile")
endif()
set(MANIFEST ${ZARC_ROOT}/config/targets/native-sim-${CONFIG_ZARC_REFERENCE_PROFILE}.json)
set(GENERATED ${CMAKE_CURRENT_BINARY_DIR}/generated/zarc_manifest.h)
add_custom_command(OUTPUT ${GENERATED}
 COMMAND ${Python3_EXECUTABLE} ${ZARC_ROOT}/tools/target_manifest.py
         ${MANIFEST} --header ${GENERATED}
 DEPENDS ${ZARC_ROOT}/tools/target_manifest.py ${MANIFEST}
 VERBATIM)
add_custom_target(zarc_manifest DEPENDS ${GENERATED})
add_dependencies(app zarc_manifest)
target_include_directories(app PRIVATE ${ZARC_ROOT}/include ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_include_directories(app PRIVATE ../adapters)
target_sources(app PRIVATE src/main.c src/provider_fixture.c src/observation_fixture.c
 ../adapters/observation_bus.c ../adapters/device_pm.c
 ${ZARC_ROOT}/src/runtime.c ${ZARC_ROOT}/src/runtime_wire.c ${ZARC_ROOT}/src/composition.c
 ${ZARC_ROOT}/src/domain.c ${ZARC_ROOT}/src/controller.c ${ZARC_ROOT}/src/connect.c
 ${ZARC_ROOT}/src/health.c ${ZARC_ROOT}/src/diagnostics.c
 ${ZARC_ROOT}/src/power.c ${ZARC_ROOT}/src/schedule.c ${ZARC_ROOT}/src/observations.c)

if(CONFIG_IPC_SERVICE)
 target_include_directories(app PRIVATE ../adapters)
 target_sources(app PRIVATE ../adapters/ipc.c src/ipc_fixture.c)
endif()
