#
# benchmarks/CMakeLists.txt - Benchmark configuration for platform module
#

# Benchmark framework library
add_library(bench_framework OBJECT
    bench_framework.c
)

target_include_directories(bench_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(bench_framework
    PUBLIC
        finkit_platform_static
)

# Benchmark executable
add_executable(platform_benchmarks
    bench_main.c
    bench_platform.c
)

# Link with benchmark framework and platform library
target_link_libraries(platform_benchmarks
    PRIVATE
        bench_framework
        finkit_platform_static
        m
)

target_compile_definitions(platform_benchmarks PRIVATE
    FC_STATIC_BUILD
)

# Include directories
target_include_directories(platform_benchmarks
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for benchmarks
target_compile_options(platform_benchmarks PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)

# Installation
install(TARGETS platform_benchmarks
    RUNTIME DESTINATION bin
)
