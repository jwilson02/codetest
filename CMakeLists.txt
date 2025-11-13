cmake_minimum_required(VERSION 3.15)
project(SCUMMStyleARPG VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs (clangd, VS Code, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    # Windows/Visual Studio
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    # Enable multi-processor compilation
    add_compile_options(/MP)
    # Disable specific warnings if needed
    # add_compile_options(/wd4251) # DLL interface warning
else()
    # GCC/Clang
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Optional: treat warnings as errors in Release builds
    if(CMAKE_BUILD_TYPE MATCHES "Release")
        # add_compile_options(-Werror)
    endif()
endif()

# Platform-specific definitions
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS)
elseif(UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif()

# Find required packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# SDL2_mixer for audio
find_package(SDL2_mixer REQUIRED)

# Optional: SDL2_image and SDL2_ttf
find_package(SDL2_image QUIET)
find_package(SDL2_ttf QUIET)

# Include directories
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# Collect all source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/Engine/*.cpp"
    "src/Engine/*.h"
)

file(GLOB_RECURSE RENDERING_SOURCES
    "src/Rendering/*.cpp"
    "src/Rendering/*.h"
)

file(GLOB_RECURSE AUDIO_SOURCES
    "src/Audio/*.cpp"
    "src/Audio/*.h"
)

file(GLOB_RECURSE UI_SOURCES
    "src/UI/*.cpp"
    "src/UI/*.h"
)

file(GLOB_RECURSE AI_SOURCES
    "src/AI/*.cpp"
    "src/AI/*.h"
)

file(GLOB_RECURSE COMBAT_SOURCES
    "src/Combat/*.cpp"
    "src/Combat/*.h"
)

file(GLOB_RECURSE RPG_SOURCES
    "src/RPG/*.cpp"
    "src/RPG/*.h"
)

file(GLOB_RECURSE QUEST_SOURCES
    "src/Quest/*.cpp"
    "src/Quest/*.h"
)

file(GLOB_RECURSE INVENTORY_SOURCES
    "src/Inventory/*.cpp"
    "src/Inventory/*.h"
)

file(GLOB_RECURSE STORY_SOURCES
    "src/Story/*.cpp"
    "src/Story/*.h"
)

file(GLOB_RECURSE WORLD_SOURCES
    "src/World/*.cpp"
    "src/World/*.h"
)

file(GLOB_RECURSE CRAFTING_SOURCES
    "src/Crafting/*.cpp"
    "src/Crafting/*.h"
)

file(GLOB_RECURSE SAVE_SOURCES
    "src/Save/*.cpp"
    "src/Save/*.h"
)

file(GLOB_RECURSE ENVIRONMENT_SOURCES
    "src/Environment/*.cpp"
    "src/Environment/*.h"
)

file(GLOB_RECURSE NETWORK_SOURCES
    "src/Network/*.cpp"
    "src/Network/*.h"
)

file(GLOB_RECURSE ASSETS_SOURCES
    "src/Assets/*.cpp"
    "src/Assets/*.h"
)

file(GLOB_RECURSE PERFORMANCE_SOURCES
    "src/Performance/*.cpp"
    "src/Performance/*.h"
)

file(GLOB_RECURSE LAUNCHER_SOURCES
    "src/Launcher/*.cpp"
    "src/Launcher/*.h"
)

# Combine all sources
set(ALL_GAME_SOURCES
    ${ENGINE_SOURCES}
    ${RENDERING_SOURCES}
    ${AUDIO_SOURCES}
    ${UI_SOURCES}
    ${AI_SOURCES}
    ${COMBAT_SOURCES}
    ${RPG_SOURCES}
    ${QUEST_SOURCES}
    ${INVENTORY_SOURCES}
    ${STORY_SOURCES}
    ${WORLD_SOURCES}
    ${CRAFTING_SOURCES}
    ${SAVE_SOURCES}
    ${ENVIRONMENT_SOURCES}
    ${NETWORK_SOURCES}
    ${ASSETS_SOURCES}
    ${PERFORMANCE_SOURCES}
    ${LAUNCHER_SOURCES}
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${ALL_GAME_SOURCES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

# GLM is header-only but we link it if it's a target
if(TARGET glm::glm)
    target_link_libraries(${PROJECT_NAME} glm::glm)
endif()

# Optional SDL2_image
if(SDL2_IMAGE_FOUND)
    target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
    add_definitions(-DUSE_SDL2_IMAGE)
endif()

# Optional SDL2_ttf
if(SDL2_TTF_FOUND)
    target_link_libraries(${PROJECT_NAME} ${SDL2_TTF_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
    add_definitions(-DUSE_SDL2_TTF)
endif()

# Platform-specific linking
if(WIN32)
    # Link SDL2main for WinMain entry point
    if(TARGET SDL2::SDL2main)
        target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)
    endif()
    # Windows-specific libraries
    target_link_libraries(${PROJECT_NAME} ws2_32 winmm)
elseif(APPLE)
    # macOS frameworks
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX)
    # Linux libraries
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to build directory"
)

# Copy data files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/bin/data
    COMMENT "Copying data files to build directory"
)

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION bin
    OPTIONAL
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/data
    DESTINATION bin
    OPTIONAL
)

# Build examples (optional)
option(BUILD_EXAMPLES "Build example programs" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tools (optional)
option(BUILD_TOOLS "Build development tools" OFF)
if(BUILD_TOOLS)
    # Add tool executables here
endif()

# Testing support
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    # Add test subdirectory here
endif()

# Print configuration summary
message(STATUS "=================================================")
message(STATUS "SCUMM Style ARPG - Build Configuration")
message(STATUS "=================================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "-------------------------------------------------")
message(STATUS "Dependencies:")
message(STATUS "  SDL2: ${SDL2_LIBRARIES}")
message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "  GLEW: ${GLEW_LIBRARIES}")
message(STATUS "  SDL2_mixer: ${SDL2_MIXER_LIBRARIES}")
if(SDL2_IMAGE_FOUND)
    message(STATUS "  SDL2_image: ${SDL2_IMAGE_LIBRARIES}")
else()
    message(STATUS "  SDL2_image: NOT FOUND (optional)")
endif()
if(SDL2_TTF_FOUND)
    message(STATUS "  SDL2_ttf: ${SDL2_TTF_LIBRARIES}")
else()
    message(STATUS "  SDL2_ttf: NOT FOUND (optional)")
endif()
message(STATUS "-------------------------------------------------")
message(STATUS "Build options:")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Tools: ${BUILD_TOOLS}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "=================================================")
