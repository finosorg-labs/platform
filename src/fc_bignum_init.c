/**
 * @file fc_bignum_init.c
 * @brief High-precision bignum subsystem initialization and lifecycle
 *
 * Manages GMP/MPFR initialization, custom memory allocators, and global state.
 */

#include "fc_bignum_internal.h"

#include "platform.h"

#include <gmp.h>
#include <mpfr.h>
#include <stdatomic.h>
#include <stdlib.h>

static atomic_int g_fc_bignum_initialized = 0;

/**
 * @brief Custom allocator for GMP/MPFR
 *
 * Uses calloc to zero-initialize memory for MemorySanitizer compatibility.
 */
static void* fc_bignum_alloc(size_t size) {
    return calloc(1, size);
}

/**
 * @brief Custom reallocator for GMP/MPFR
 */
static void* fc_bignum_realloc(void* ptr, size_t old_size, size_t new_size) {
    (void) old_size;
    return realloc(ptr, new_size);
}

/**
 * @brief Custom deallocator for GMP/MPFR
 */
static void fc_bignum_free(void* ptr, size_t size) {
    (void) size;
    free(ptr);
}

fc_status_t fc_bignum_global_init(void) {
    int expected = 0;
    if (!atomic_compare_exchange_strong(&g_fc_bignum_initialized, &expected, 1)) {
        return FC_OK;
    }

    mp_set_memory_functions(fc_bignum_alloc, fc_bignum_realloc, fc_bignum_free);
    fc_precision_apply_defaults_to_mpfr();
    mpfr_clear_flags();
    return FC_OK;
}

void fc_bignum_global_cleanup(void) {
    mpfr_free_cache();
    mpfr_clear_flags();
    atomic_store(&g_fc_bignum_initialized, 0);
}

int fc_bignum_is_initialized(void) {
    return atomic_load(&g_fc_bignum_initialized) != 0;
}

fc_rounding_mode_t fc_bignum_current_rounding_mode(void) {
    return fc_precision_get_rounding_mode_internal();
}
