cmake_minimum_required(VERSION 3.15)
project(FC3D C)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

if (EMSCRIPTEN)
    set(PLATFORM Web)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    add_definitions(-DPLATFORM_WEB)
    add_definitions(-DUSE_WASM)

    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY index.html DESTINATION ${CMAKE_BINARY_DIR}/)
endif ()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (EMSCRIPTEN)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} 
        PROPERTIES SUFFIX ".js"
        LINK_FLAGS "--bind -s WASM=1 -s USE_GLFW=3 -s USE_WEBGL2=1 -s FORCE_FILESYSTEM=1 --memory-init-file 0 -s INITIAL_MEMORY=64MB -s SAFE_HEAP=1 --use-preload-plugins --embed-file assets/")
endif ()


add_subdirectory(vendor/raylib)
add_subdirectory(vendor/lua)

target_include_directories(${PROJECT_NAME} PUBLIC SYSTEM vendor/raylib/src vendor/lua/)
target_link_libraries(${PROJECT_NAME} raylib lua)

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
