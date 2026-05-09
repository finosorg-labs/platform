/**
 * @file bench_platform.c
 * @brief platform module benchmark entry point
 *
 * This file serves as the main benchmark registration point for the platform module.
 * Individual benchmark modules are in separate files:
 */

#include "bench_framework.h"
#include <simd_detect.h>
#include <stdio.h>

/* External benchmark functions from sub-modules */

/* Entry point for platform benchmarks */
void bench_platform_run(void) {
    printf("\n");
    printf("============================================================\n");
    printf("  Codec Module Performance Benchmarks\n");
    printf("  SIMD level: %s\n", fc_simd_level_string(fc_detect_simd()));
    printf("============================================================\n");

    /* Run all sub-module benchmarks */

    printf("\n");
    printf("============================================================\n");
    printf("  Codec benchmarks complete\n");
    printf("============================================================\n");
}
