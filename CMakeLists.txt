################################################################################
## The configuration files for the tk-window project. This requires CMake 
## version 3.10 or higher. The following options effect this file's output:
##
## TKWIN_UNDERLYING<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
##
## Copyright (c) 2025 - RPGtk Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("tkwindow" LANGUAGES C VERSION 0.0.0.8)

# We only support Linux. Everything else can go kick rocks.
if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "This operating system is not supported by tk-window.")
endif()

option(TKWIN_UNDERLYING "Toggle Wayland (ON) or X11 (OFF) mode." ON)

# https://en.wikipedia.org/wiki/C23_(C_standard_revision)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(TKWIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(TKWIN_SOURCES "${TKWIN_SOURCE_DIR}/TKWindow.c")
if(${TKWIN_UNDERLYING})
    list(APPEND TKWIN_SOURCES "${TKWIN_SOURCE_DIR}/Targets/Wayland.c")
else()
    list(APPEND TKWIN_SOURCES "${TKWIN_SOURCE_DIR}/Targets/X11.c")
endif()

add_library(${PROJECT_NAME} STATIC ${TKWIN_SOURCES})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Modules")
target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/Include")

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror -Wpedantic -Wextra)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    # https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    target_compile_options(${PROJECT_NAME} PUBLIC -Og -g3 -ggdb -fsanitize=address 
        -fsanitize=pointer-compare  -fsanitize=leak -fsanitize=pointer-subtract 
        -fsanitize=undefined)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address 
        -fsanitize=undefined)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html
        target_compile_options(${PROJECT_NAME} PUBLIC -fanalyzer)
    endif()
else()
    # https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#index-march-15
    # https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#index-mtune-17
    target_compile_options(${PROJECT_NAME} PUBLIC -march=native -mtune=native 
        -Ofast -flto)
    target_link_options(${PROJECT_NAME} PUBLIC -Ofast -flto)
endif()

if(${TKWIN_UNDERLYING})
    target_compile_definitions(${PROJECT_NAME} PRIVATE WAYLAND)

    find_package(Wayland REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${Wayland_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Wayland_INCLUDE_DIRS})

    find_package(Scanner REQUIRED)
    ecm_add_wayland_client_protocol(${PROJECT_NAME} PROTOCOL 
        "${CMAKE_CURRENT_SOURCE_DIR}/Modules/Protocols/xdg-shell.xml" BASENAME 
        xdg-shell PRIVATE_CODE)
    target_include_directories(${PROJECT_NAME} PUBLIC 
        "${CMAKE_CURRENT_BINARY_DIR}")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE X11)
    # TODO: X11.
endif()
