################################################################################
## The configuration files for the TKWindow project. This requires CMake 
## version 3.10 or higher.
##
## Copyright (c) 2025 - RPGtk Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("TKWindow" LANGUAGES C VERSION 0.0.0.15)

include(Helpers)
run_checks(ON)
create_target(OFF)

if(${TK_UNDERLYING})
    target_compile_definitions(${PROJECT_NAME} PRIVATE WAYLAND)

    find_package(Wayland REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${Wayland_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${Wayland_INCLUDE_DIRS})
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE X11)
    # TODO: X11.
endif()
