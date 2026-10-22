cmake_minimum_required(VERSION 3.12)

# The pico "OS" (SDK bootstrap, shared build flags, library set) lives in the
# os/ submodule. See os/pico_os.cmake.
include(${CMAKE_CURRENT_LIST_DIR}/os/pico_os.cmake)

pico_os_pre_project()

project(irrigation C CXX ASM)

pico_os_init()

set(APP_SOURCES
  src/main.cpp
  src/irrigation_manager.cpp
  )

# pico_common brings the OS; add hardware_* and optional os libraries as needed.
set(APP_LIBRARIES
  pico_common
  pico_telegram
  pico_calendar_commands
  )

add_executable(irrigation ${APP_SOURCES})

# The web page, built from src/web/page.yaml (see os/web/README.md).
pico_os_web_page(irrigation ${CMAKE_CURRENT_SOURCE_DIR}/src/web/page.yaml)

# A real 16K stack for core 0, reserved and checked by the linker; the scratch
# banks move down and RAM shortens to match. Chip independent: the addresses
# come from the SDK defaults for the board being built.
#
# PICO_CORE1_STACK_SIZE is not set: pico_multicore is not linked, core 1 is
# left parked in the bootrom, and .stack1 is empty.
pico_os_set_stack_size(irrigation 16384)

# Settings up to 4K: the telegram and calendar secrets make them bigger than the 2K default.
target_compile_definitions(irrigation PRIVATE
  PICO_STDIO_STACK_BUFFER_SIZE=1024
  PICO_MALLOC_PANIC=1
  CONFIG_MAX_JSON=4096
  MAX_BUFFERED_COMMAND_BYTES=4096
  )

# Sources and the app's own config/ (the page's header: pico_os_web_page above).
target_include_directories(irrigation PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config
  )

target_link_libraries(irrigation ${APP_LIBRARIES})

# Every command in src/commands/, plus the os's own (config, upgrade, reboot,
# ...), telegram's /start and the calendar status, for mqtt, the page
# websocket, the chat and calendar events.
pico_commands_add(irrigation
  ${CMAKE_CURRENT_SOURCE_DIR}/src/commands
  ${PICO_BUILTIN_COMMANDS}
  ${PICO_SIMPLE_CONFIG_COMMANDS}
  ${PICO_TELEGRAM_COMMANDS}
  ${PICO_CALENDAR_COMMANDS}
  )

pico_add_extra_outputs(irrigation)

pico_enable_stdio_usb(irrigation 0)
pico_enable_stdio_uart(irrigation 0)
