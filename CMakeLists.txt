################################################################################
## The configuration files for the Hyacinth project. This requires CMake version 
## 3.10 or higher.
##
## Copyright (c) 2025 - the Waterlily Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Hyacinth" LANGUAGES C VERSION 0.0.0.43)

include(Helpers)
run_checks(ON)

create_target(OFF RAW_SOURCES "${WATERLILY_UNDERLYING_SYSTEM}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ${WATERLILY_UNDERLYING_SYSTEM_UC})
link_external(${WATERLILY_UNDERLYING_SYSTEM})

add_dependencies(${PROJECT_NAME} Primrose)
target_include_directories(${PROJECT_NAME} PRIVATE "$<TARGET_PROPERTY:Primrose,INTERFACE_INCLUDE_DIRECTORIES>")
