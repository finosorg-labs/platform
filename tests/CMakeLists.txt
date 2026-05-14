#
# tests/CMakeLists.txt - Test configuration for platform module
#

# Test executable
add_executable(platform_tests
    test_framework.c
    test_main.c
    test_platform.c
    test_bigint.c
    test_bigfloat.c
    test_precision.c
)

# Link with platform library
target_link_libraries(platform_tests
    PRIVATE
        finkit_platform_static_base
        ${FC_MPFR_STATIC_LIB}
        ${FC_GMP_STATIC_LIB}
        m
)

target_compile_definitions(platform_tests PRIVATE
    FC_STATIC_BUILD
)

# Include directories
target_include_directories(platform_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for tests
target_compile_options(platform_tests PRIVATE -Wall -Wextra -Wpedantic)

# Run tests with ctest
add_test(
    NAME "platform_tests"
    COMMAND platform_tests
)

# Installation
install(TARGETS platform_tests
    RUNTIME DESTINATION bin
)
