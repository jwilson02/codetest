cmake_minimum_required(VERSION 3.15)
project(SCUMM_VM_Story_System VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test suite" OFF)
option(ENABLE_LOCALIZATION "Enable localization support" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Performance System Library
add_library(Performance
    Performance/Profiler.cpp
    Performance/MemoryTracker.cpp
    Performance/ObjectPool.cpp
    Performance/SpatialPartition.cpp
    Performance/ThreadPool.cpp
)

target_include_directories(Performance
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Link pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(Performance PUBLIC Threads::Threads)

# Compiler warnings for Performance
if(MSVC)
    target_compile_options(Performance PRIVATE /W4)
else()
    target_compile_options(Performance PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Enable profiling and memory tracking in debug builds
option(ENABLE_PROFILING "Enable performance profiling" ON)
option(ENABLE_MEMORY_TRACKING "Enable memory tracking" ON)

if(ENABLE_PROFILING)
    target_compile_definitions(Performance PUBLIC PROFILING_ENABLED)
endif()

if(ENABLE_MEMORY_TRACKING)
    target_compile_definitions(Performance PUBLIC MEMORY_TRACKING_ENABLED)
endif()

# Story System Library
add_library(Story
    Story/DialogueSystem.cpp
    Story/DialogueTree.cpp
    Story/NarrativeEngine.cpp
    Story/RelationshipSystem.cpp
    Story/Journal.cpp
    Story/Cutscene.cpp
)

target_include_directories(Story
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(Story PRIVATE /W4)
else()
    target_compile_options(Story PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Example executable (optional)
if(BUILD_EXAMPLES)
    add_executable(StorySystemDemo
        ../examples/demo.cpp
    )
    target_link_libraries(StorySystemDemo PRIVATE Story)
endif()

# Install rules
install(TARGETS Performance Story
    EXPORT StoryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY Performance/
    DESTINATION include/Performance
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY Story/
    DESTINATION include/Story
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ../data/
    DESTINATION share/Story/data
)

# Print configuration
message(STATUS "")
message(STATUS "SCUMM VM Story System Configuration:")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples:       ${BUILD_EXAMPLES}")
message(STATUS "  Build Tests:          ${BUILD_TESTS}")
message(STATUS "  Enable Localization:  ${ENABLE_LOCALIZATION}")
message(STATUS "")
