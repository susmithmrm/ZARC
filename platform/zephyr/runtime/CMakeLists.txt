cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zarc_runtime_reference LANGUAGES C)
set(ZARC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(GENERATED ${CMAKE_CURRENT_BINARY_DIR}/generated/zarc_manifest.h)
add_custom_command(OUTPUT ${GENERATED}
 COMMAND ${Python3_EXECUTABLE} ${ZARC_ROOT}/tools/target_manifest.py
         ${ZARC_ROOT}/config/targets/native-sim-edge.json --header ${GENERATED}
 DEPENDS ${ZARC_ROOT}/tools/target_manifest.py ${ZARC_ROOT}/config/targets/native-sim-edge.json
 VERBATIM)
add_custom_target(zarc_manifest DEPENDS ${GENERATED})
add_dependencies(app zarc_manifest)
target_include_directories(app PRIVATE ${ZARC_ROOT}/include ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_sources(app PRIVATE src/main.c
 ${ZARC_ROOT}/src/runtime.c ${ZARC_ROOT}/src/runtime_wire.c ${ZARC_ROOT}/src/composition.c
 ${ZARC_ROOT}/src/domain.c ${ZARC_ROOT}/src/controller.c ${ZARC_ROOT}/src/connect.c
 ${ZARC_ROOT}/src/health.c ${ZARC_ROOT}/src/diagnostics.c
 ${ZARC_ROOT}/src/schedule.c ${ZARC_ROOT}/src/observations.c)
