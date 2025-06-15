################################################################################
## The configuration files for the TKWindow project. This requires CMake 
## version 3.10 or higher. The following options effect this file's output:
##
## TK_UNDERLYING<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
##
## Copyright (c) 2025 - RPGtk Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("TKWindow" LANGUAGES C VERSION 0.0.0.12)

include(Helpers)
run_checks()
create_target(OFF)

if(${TK_UNDERLYING})
    target_compile_definitions(${PROJECT_NAME} PRIVATE WAYLAND)

    find_package(Wayland REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${Wayland_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${Wayland_INCLUDE_DIRS})

    find_package(Scanner REQUIRED)
    ecm_add_wayland_client_protocol(${PROJECT_NAME} PROTOCOL 
        "${CMAKE_CURRENT_SOURCE_DIR}/Protocols/xdg-shell.xml" BASENAME 
        xdg-shell PRIVATE_CODE)
    target_include_directories(${PROJECT_NAME} PRIVATE 
        "${CMAKE_CURRENT_BINARY_DIR}")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE X11)
    # TODO: X11.
endif()
