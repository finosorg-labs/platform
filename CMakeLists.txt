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

# Try to find third-party libraries in build directory first, then fall back to source directory
if(EXISTS ${CMAKE_BINARY_DIR}/third_party/gmp/libgmp.a)
    set(FC_GMP_STATIC_LIB ${CMAKE_BINARY_DIR}/third_party/gmp/libgmp.a)
else()
    set(FC_GMP_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gmp/.libs/libgmp.a)
endif()

if(EXISTS ${CMAKE_BINARY_DIR}/third_party/mpfr/libmpfr.a)
    set(FC_MPFR_STATIC_LIB ${CMAKE_BINARY_DIR}/third_party/mpfr/libmpfr.a)
else()
    set(FC_MPFR_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mpfr/src/.libs/libmpfr.a)
endif()

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
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gmp/.libs
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mpfr/src/.libs
)

target_compile_definitions(finkit_platform PRIVATE FC_STATIC_BUILD)

# Determine platform-specific build directory
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_BUILD_DIR "linux_amd64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(PLATFORM_BUILD_DIR "darwin_arm64")
    else()
        set(PLATFORM_BUILD_DIR "darwin_amd64")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_BUILD_DIR "windows_amd64")
endif()

# Create base static library (without third-party libraries merged) in CMake build directory
add_library(finkit_platform_static_base STATIC $<TARGET_OBJECTS:finkit_platform>)
target_include_directories(finkit_platform_static_base
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
# Get the actual path where CMake will place the base library
set(BASE_LIB_PATH $<TARGET_FILE:finkit_platform_static_base>)

# Merge static libraries into a single library
set(MERGED_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM_BUILD_DIR}/libfinkit_platform_static.a)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows: use lib.exe to merge libraries
    add_custom_command(
        OUTPUT ${MERGED_LIB_PATH}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM_BUILD_DIR}
        COMMAND lib.exe /OUT:${MERGED_LIB_PATH}
            $<TARGET_FILE:finkit_platform_static_base>
            ${FC_GMP_STATIC_LIB}
            ${FC_MPFR_STATIC_LIB}
        DEPENDS finkit_platform_static_base ${FC_GMP_STATIC_LIB} ${FC_MPFR_STATIC_LIB}
        COMMENT "Merging static libraries into ${MERGED_LIB_PATH}"
        VERBATIM
    )
else()
    # Unix/Linux/macOS: use ar with MRI script to merge libraries
    set(MRI_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/merge_libs.mri)
    file(GENERATE OUTPUT ${MRI_SCRIPT} CONTENT
"CREATE ${MERGED_LIB_PATH}
ADDLIB $<TARGET_FILE:finkit_platform_static_base>
ADDLIB ${FC_GMP_STATIC_LIB}
ADDLIB ${FC_MPFR_STATIC_LIB}
SAVE
END
")

    add_custom_command(
        OUTPUT ${MERGED_LIB_PATH}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM_BUILD_DIR}
        COMMAND ${CMAKE_AR} -M < ${MRI_SCRIPT}
        DEPENDS finkit_platform_static_base ${FC_GMP_STATIC_LIB} ${FC_MPFR_STATIC_LIB} ${MRI_SCRIPT}
        COMMENT "Merging static libraries into ${MERGED_LIB_PATH}"
        VERBATIM
    )
endif()

# Create custom target to trigger the merge
add_custom_target(finkit_platform_static ALL
    DEPENDS ${MERGED_LIB_PATH}
    COMMAND ${CMAKE_COMMAND} -E echo "Merged library created at: ${MERGED_LIB_PATH}"
)

# Create interface library for linking
add_library(finkit_platform_static_interface INTERFACE)
target_include_directories(finkit_platform_static_interface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(finkit_platform_static_interface
    INTERFACE
        m
        ${MERGED_LIB_PATH}
)

# Create alias
add_library(finkit::platform ALIAS finkit_platform_static_interface)

# Install merged static library
install(FILES ${MERGED_LIB_PATH}
    DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/platform
    FILES_MATCHING PATTERN "*.h"
)

# Install interface library target
install(TARGETS finkit_platform_static_interface
    EXPORT finkit-platform-targets
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
    add_subdirectory(tests)
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
