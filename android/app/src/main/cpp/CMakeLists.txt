cmake_minimum_required(VERSION 3.22.1)

project(minigame_android C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../external/raylib")
set(PLATFORM Android CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RSHAPES ON CACHE BOOL "" FORCE)
add_subdirectory(${RAYLIB_DIR} ${CMAKE_BINARY_DIR}/raylib-build)

add_library(main SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    ../../../../../platformer.c
)

target_compile_definitions(main PRIVATE PLATFORM_ANDROID=1)
target_include_directories(main PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
)
target_link_libraries(main PRIVATE raylib android log EGL GLESv2 m dl)
