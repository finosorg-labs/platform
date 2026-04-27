cmake_minimum_required(VERSION 3.16)
project(finkit-platform VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic -ffp-contract=off)

# Sanitizer configuration
if(FC_ENABLE_SANITIZERS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.github/sanitizers.cmake)
        include(${CMAKE_CURRENT_SOURCE_DIR}/.github/sanitizers.cmake)
    endif()
endif()

# Platform-specific sources (selected by TARGET OS, not host OS)
set(PLATFORM_SOURCES
    src/simd_detect.c
    src/mem_aligned.c
    src/error.c
    src/fc_init.c
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SOURCES src/platform_win.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND PLATFORM_SOURCES src/platform_macos.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PLATFORM_SOURCES src/platform_linux.c)
endif()

# Create object library
add_library(finkit_platform OBJECT ${PLATFORM_SOURCES})

target_include_directories(finkit_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(finkit_platform PRIVATE FC_STATIC_BUILD)

# Create static library
add_library(finkit_platform_static STATIC $<TARGET_OBJECTS:finkit_platform>)
target_include_directories(finkit_platform_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link math library
target_link_libraries(finkit_platform_static PUBLIC m)

# Create alias
add_library(finkit::platform ALIAS finkit_platform_static)

# Install
install(TARGETS finkit_platform_static
    EXPORT finkit-platform-targets
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/platform
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT finkit-platform-targets
    FILE finkit-platformTargets.cmake
    NAMESPACE finkit::
    DESTINATION lib/cmake/finkit-platform
)

# Configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/finkit-platformConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/finkit-platformConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/finkit-platformConfig.cmake"
    INSTALL_DESTINATION lib/cmake/finkit-platform
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/finkit-platformConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/finkit-platformConfigVersion.cmake"
    DESTINATION lib/cmake/finkit-platform
)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    # add_subdirectory(tests)
endif()

# Print configuration
message(STATUS "")
message(STATUS "============================================================")
message(STATUS " finkit-platform ${PROJECT_VERSION}")
message(STATUS "============================================================")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "============================================================")
message(STATUS "")
