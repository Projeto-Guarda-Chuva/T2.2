cmake_minimum_required(VERSION 3.10)
project(audio_actuator C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0 gthread-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-pbutils-1.0)

find_library(MICROHTTPD_LIB microhttpd REQUIRED)
find_library(JSONC_LIB json-c REQUIRED)
find_path(JSONC_INCLUDE_DIR json-c/json.h REQUIRED)

include_directories(
    main
    ${GLIB_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${JSONC_INCLUDE_DIR}
)

add_executable(audio_actuator 
    main/main.c 
    main/audio_manager.c 
    main/server.c
    main/audio_manager.h
    main/server.h
)

target_link_libraries(audio_actuator 
    ${GLIB_LIBRARIES} 
    ${GSTREAMER_LIBRARIES} 
    ${MICROHTTPD_LIB}
    ${JSONC_LIB}
)

target_compile_definitions(audio_actuator PRIVATE PRODUCTION_ENV)