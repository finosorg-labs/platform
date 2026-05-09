cmake_minimum_required(VERSION 3.16)
project(finkit-platform VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include SIMD detection and compiler flags
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DetectSIMD.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerFlags.cmake)

# Sanitizer configuration
if(FC_ENABLE_SANITIZERS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake)
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake)
    endif()
endif()

# Generate cgo_source.go with detected SIMD flags
if(FC_SIMD_LEVEL STREQUAL "AVX512")
    set(FC_SIMD_CFLAGS "${FC_AVX512_FLAGS}")
elseif(FC_SIMD_LEVEL STREQUAL "AVX2")
    set(FC_SIMD_CFLAGS "${FC_AVX2_FLAGS}")
elseif(FC_SIMD_LEVEL STREQUAL "SSE42")
    set(FC_SIMD_CFLAGS "${FC_SSE42_FLAGS}")
else()
    set(FC_SIMD_CFLAGS "")
endif()
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cgo_source.go.in
    ${CMAKE_CURRENT_SOURCE_DIR}/cgo_source.go
    @ONLY
)

# Platform-specific sources (selected by TARGET OS, not host OS)
set(PLATFORM_SOURCES
    src/simd_detect.c
    src/mem_aligned.c
    src/error.c
    src/fc_init.c
    src/fc_bignum_init.c
    src/fc_bigint.c
    src/fc_bigfloat.c
    src/fc_precision.c
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SOURCES src/platform_win.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND PLATFORM_SOURCES src/platform_macos.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PLATFORM_SOURCES src/platform_linux.c)
endif()

set(FC_GMP_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gmp/.libs/libgmp.a)
set(FC_MPFR_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mpfr/src/.libs/libmpfr.a)

# Create object library
add_library(finkit_platform OBJECT ${PLATFORM_SOURCES})

target_include_directories(finkit_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mpfr/src
)

target_compile_definitions(finkit_platform PRIVATE FC_STATIC_BUILD)

# Create static library
add_library(finkit_platform_static STATIC $<TARGET_OBJECTS:finkit_platform>)
target_include_directories(finkit_platform_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link third-party and math libraries
target_link_libraries(finkit_platform_static
    PUBLIC
        m
        ${FC_MPFR_STATIC_LIB}
        ${FC_GMP_STATIC_LIB}
)

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
