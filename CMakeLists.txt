cmake_minimum_required(VERSION 3.15)
project(FC3D C)

set(CMAKE_C_STANDARD 11)

if (EMSCRIPTEN)
    set(PLATFORM Web)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    add_definitions(-DPLATFORM_WEB)
    add_definitions(-DUSE_WASM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --bind --memory-init-file 0 -s INITIAL_MEMORY=64MB -s SAFE_HEAP=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s USE_WEBGL2=1 -s FORCE_FILESYSTEM=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s WASM=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ASSERTIONS=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --use-preload-plugins")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --embed-file assets/")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY index.html DESTINATION ${CMAKE_BINARY_DIR}/)
endif ()

file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

add_subdirectory(vendor/raylib)

target_include_directories(${PROJECT_NAME} PUBLIC SYSTEM vendor/raylib/src)
target_link_libraries(${PROJECT_NAME} raylib)

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
