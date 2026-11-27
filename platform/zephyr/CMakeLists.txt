cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zarc_native_lab LANGUAGES C)
set(ZARC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_include_directories(app PRIVATE ${ZARC_ROOT}/include)
target_sources(app PRIVATE src/main.c
  ${ZARC_ROOT}/src/domain.c
  ${ZARC_ROOT}/src/connect.c
  ${ZARC_ROOT}/src/controller.c
  ${ZARC_ROOT}/src/composition.c
  ${ZARC_ROOT}/src/health.c
  ${ZARC_ROOT}/src/diagnostics.c
  ${ZARC_ROOT}/src/config_store.c
  ${ZARC_ROOT}/src/power.c
  ${ZARC_ROOT}/src/update.c)

add_subdirectory(adapters)
