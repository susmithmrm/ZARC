# Selected board application enables only the approved native providers.
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR})
if(CONFIG_WATCHDOG)
  target_sources(app PRIVATE watchdog.c)
endif()
if(CONFIG_NVS)
  target_sources(app PRIVATE storage.c)
endif()
