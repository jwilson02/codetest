# Audio System CMake Configuration

cmake_minimum_required(VERSION 3.10)

# Audio System Library
add_library(AudioSystem STATIC
    AudioSystem.cpp
    AudioSystem.h
    MusicPlayer.cpp
    MusicPlayer.h
    SoundEffect.cpp
    SoundEffect.h
    AudioSource.cpp
    AudioSource.h
)

# Find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Include directories
target_include_directories(AudioSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(AudioSystem PUBLIC
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

# If using pkg-config instead of find_package
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SDL2 REQUIRED sdl2)
# pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# Set C++ standard
target_compile_features(AudioSystem PUBLIC cxx_std_17)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(AudioSystem PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    # macOS-specific settings
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    target_link_libraries(AudioSystem PRIVATE
        ${COREAUDIO_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
    )
elseif(UNIX)
    # Linux-specific settings
    target_link_libraries(AudioSystem PRIVATE m pthread)
endif()

# Optional: JSON support for configuration loading
# Uncomment if you're using nlohmann/json or similar
# find_package(nlohmann_json REQUIRED)
# target_link_libraries(AudioSystem PRIVATE nlohmann_json::nlohmann_json)

# Installation
install(TARGETS AudioSystem
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    AudioSystem.h
    MusicPlayer.h
    SoundEffect.h
    AudioSource.h
    DESTINATION include/Audio
)
