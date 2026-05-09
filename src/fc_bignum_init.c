#include "fc_bignum_internal.h"

#include "platform.h"

#include <gmp.h>
#include <mpfr.h>
#include <stdatomic.h>
#include <stdlib.h>

#define FC_BIGFLOAT_DEFAULT_PRECISION 256

static atomic_int g_fc_bignum_initialized       = 0;
static atomic_ulong g_fc_default_precision_bits = FC_BIGFLOAT_DEFAULT_PRECISION;
static atomic_int g_fc_default_rounding_mode    = FC_RNDN;

static void* fc_bignum_alloc(size_t size) {
    return malloc(size);
}

static void* fc_bignum_realloc(void* ptr, size_t old_size, size_t new_size) {
    (void) old_size;
    return realloc(ptr, new_size);
}

static void fc_bignum_free(void* ptr, size_t size) {
    (void) size;
    free(ptr);
}

static mpfr_rnd_t fc_bignum_to_mpfr_rounding(fc_rounding_mode_t mode) {
    switch (mode) {
    case FC_RNDN:
        return MPFR_RNDN;
    case FC_RNDZ:
        return MPFR_RNDZ;
    case FC_RNDU:
        return MPFR_RNDU;
    case FC_RNDD:
        return MPFR_RNDD;
    case FC_RNDA:
        return MPFR_RNDA;
    default:
        return MPFR_RNDN;
    }
}

fc_status_t fc_bignum_global_init(void) {
    int expected = 0;
    if (!atomic_compare_exchange_strong(&g_fc_bignum_initialized, &expected, 1)) {
        return FC_OK;
    }

    mp_set_memory_functions(fc_bignum_alloc, fc_bignum_realloc, fc_bignum_free);
    mpfr_set_default_prec((mpfr_prec_t) atomic_load(&g_fc_default_precision_bits));
    mpfr_set_default_rounding_mode(fc_bignum_to_mpfr_rounding((fc_rounding_mode_t
    ) atomic_load(&g_fc_default_rounding_mode)));
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
    return (fc_rounding_mode_t) atomic_load(&g_fc_default_rounding_mode);
}

fc_status_t fc_precision_set_default_bits(fc_uint64_t bits) {
    if (bits == 0 || bits > (fc_uint64_t) MPFR_PREC_MAX) {
        return FC_ERR_INVALID_ARG;
    }

    atomic_store(&g_fc_default_precision_bits, (unsigned long) bits);
    if (fc_bignum_is_initialized()) {
        mpfr_set_default_prec((mpfr_prec_t) bits);
    }
    return FC_OK;
}

fc_status_t fc_precision_get_default_bits(fc_uint64_t* bits_out) {
    if (bits_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    *bits_out = (fc_uint64_t) atomic_load(&g_fc_default_precision_bits);
    return FC_OK;
}

fc_status_t fc_precision_set_rounding_mode(fc_rounding_mode_t mode) {
    if (mode < FC_RNDN || mode > FC_RNDA) {
        return FC_ERR_INVALID_ARG;
    }

    atomic_store(&g_fc_default_rounding_mode, mode);
    if (fc_bignum_is_initialized()) {
        mpfr_set_default_rounding_mode(fc_bignum_to_mpfr_rounding(mode));
    }
    return FC_OK;
}

fc_status_t fc_precision_get_rounding_mode(fc_rounding_mode_t* mode_out) {
    if (mode_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    *mode_out = (fc_rounding_mode_t) atomic_load(&g_fc_default_rounding_mode);
    return FC_OK;
}
