cmake_minimum_required(VERSION 3.25)

# CubeForge TogetherSpawn Mod - Root CMake Configuration
project(CubeForgeTogetherSpawn VERSION 1.0.0 LANGUAGES C CXX)

# Enforce standard-compliant and modern MSVC runtime behavior
cmake_policy(SET CMP0091 NEW)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Enable compiler cache if available (ccache / sccache)
find_program(CCACHE_PROGRAM NAMES ccache sccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Found compiler cache: ${CCACHE_PROGRAM}")
endif()

# Enable Link-Time Optimization (LTO) for optimized builds
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported LANGUAGES C CXX)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
endif()

# Compiler Interface Flags Target
add_library(cubeforge_mod_compiler_flags INTERFACE)
add_library(CubeForge::CompilerFlags ALIAS cubeforge_mod_compiler_flags)

target_compile_features(cubeforge_mod_compiler_flags INTERFACE cxx_std_20 c_std_11)

if(MSVC)
    target_compile_options(cubeforge_mod_compiler_flags INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:
            /W4             # High warning level
            /EHsc           # Standard C++ exception handling
            /utf-8          # Force UTF-8 character encoding
            /permissive-    # Enforce standard conformance
            /Zc:__cplusplus # Correct __cplusplus macro
            /wd4828         # Ignore non-representable characters warning
            /wd4100         # Suppress unreferenced formal parameter (common in hooks)
            /MP             # Multi-processor compilation
            /Zc:inline      # Remove unreferenced functions
        >
    )
    target_compile_definitions(cubeforge_mod_compiler_flags INTERFACE
        _ITERATOR_DEBUG_LEVEL=0     # Required: Match Cube World release runtime ABI
        _CRT_SECURE_NO_WARNINGS     # Suppress deprecated CRT warnings
        DIRECTINPUT_VERSION=0x0800  # DirectInput 8 compatibility
        NOMINMAX                    # Prevent Windows min/max macros collisions
        WIN32_LEAN_AND_MEAN         # Exclude rarely-used Windows headers
        MODLOADER=1                 # Marker definition for mod builds
    )
else()
    target_compile_options(cubeforge_mod_compiler_flags INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
        >
    )
endif()

# -----------------------------------------------------------------------------
# CubeForge SDK Resolution (Local sibling directory or remote FetchContent)
# -----------------------------------------------------------------------------
include(FetchContent)

if(CUBEFORGE_SDK_PATH AND EXISTS "${CUBEFORGE_SDK_PATH}/CMakeLists.txt")
    message(STATUS "Using CubeForge SDK from custom path: ${CUBEFORGE_SDK_PATH}")
    FetchContent_Declare(
        cubeforge_sdk
        SYSTEM
        SOURCE_DIR "${CUBEFORGE_SDK_PATH}"
    )
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../cubeforge.sdk/CMakeLists.txt")
    message(STATUS "Found local CubeForge SDK at ../cubeforge.sdk")
    FetchContent_Declare(
        cubeforge_sdk
        SYSTEM
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cubeforge.sdk"
    )
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CWSDK/CMakeLists.txt")
    message(STATUS "Found local CubeForge SDK at ../CWSDK")
    FetchContent_Declare(
        cubeforge_sdk
        SYSTEM
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../CWSDK"
    )
else()
    message(STATUS "Downloading CubeForge SDK via FetchContent (GitHub)...")
    FetchContent_Declare(
        cubeforge_sdk
        SYSTEM
        GIT_REPOSITORY https://github.com/Gildofj/cubeforge.sdk.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()

FetchContent_MakeAvailable(cubeforge_sdk)

# -----------------------------------------------------------------------------
# Mod DLL Target Definition
# -----------------------------------------------------------------------------
set(MOD_TARGET_NAME "TogetherSpawn")

file(GLOB_RECURSE MOD_SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_library(${MOD_TARGET_NAME} SHARED ${MOD_SOURCES})

target_link_libraries(${MOD_TARGET_NAME} PRIVATE
    cubeforge_mod_compiler_flags
    CWSDK
)

target_include_directories(${MOD_TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${cubeforge_sdk_SOURCE_DIR}
)

if(MSVC)
    set_property(TARGET ${MOD_TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Target Output Properties (Artifacts placed in dist/Mods)
set_target_properties(${MOD_TARGET_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "${MOD_TARGET_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/Mods"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/dist/Mods"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/dist/Mods"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/dist/Mods"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/Mods"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/Mods"
)

# Optional installation target into Cube World game directory
if(CUBEWORLD_PATH)
    install(TARGETS ${MOD_TARGET_NAME}
        RUNTIME DESTINATION "${CUBEWORLD_PATH}/Mods"
    )
endif()
