cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
project("awin" LANGUAGES C VERSION 0.0.0.2)

include(CMakeDependentOption)

cmake_dependent_option(AWIN_LINUX_WINDOWING "Toggle Wayland (ON) or X11 (OFF) mode." ON "UNIX;NOT APPLE" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Modules")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Tests")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror -Wpedantic -Wextra)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_compile_options(-Og -g3 -ggdb -fsanitize=address -fsanitize=pointer-compare 
        -fsanitize=leak -fsanitize=pointer-subtract -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fanalyzer)
    endif()
else()
    add_compile_options(-march=native -mtune=native -Ofast -flto)
    add_link_options(-Ofast -flto)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Target")
set(AWIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.c" "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h")
if(${AWIN_LINUX_WINDOWING})
    list(APPEND AWIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Target/wayland.c" "${CMAKE_CURRENT_SOURCE_DIR}/Target/wayland.h")
else()
    list(APPEND AWIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Target/x11.c" "${CMAKE_CURRENT_SOURCE_DIR}/Target/x11.h")
    target_compile_definitions(${PROJECT_NAME} PRIVATE X11=false)
endif()

add_library(${PROJECT_NAME} STATIC ${AWIN_SOURCES})
if(${AWIN_LINUX_WINDOWING})
    target_compile_definitions(${PROJECT_NAME} PRIVATE WAYLAND)
    find_package(Wayland REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${Wayland_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Wayland_INCLUDE_DIRS})

    find_package(Scanner REQUIRED)
    ecm_add_wayland_client_protocol(${PROJECT_NAME} PROTOCOL 
        "${CMAKE_CURRENT_SOURCE_DIR}/Modules/Protocols/xdg-shell.xml" BASENAME 
        xdg-shell PRIVATE_CODE)
    # The above function adds the header into the build directory. We need to
    # make sure we can wrangle said header.
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE X11)
endif()

file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*")
foreach(file ${TEST_FILES})
    cmake_path(GET file STEM FILE_NAME)
    add_executable(${FILE_NAME} ${file})
    target_link_libraries(${FILE_NAME} ${PROJECT_NAME})
endforeach()
