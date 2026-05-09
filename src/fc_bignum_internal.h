#ifndef FC_BIGNUM_INTERNAL_H
#define FC_BIGNUM_INTERNAL_H

#include "error.h"
#include "fc_precision.h"

#include <gmp.h>
#include <mpfr.h>

typedef struct fc_bigint {
    mpz_t value;
} fc_bigint_impl_t;

typedef struct fc_bigfloat {
    mpfr_t value;
} fc_bigfloat_impl_t;

fc_status_t fc_bignum_global_init(void);
void fc_bignum_global_cleanup(void);
int fc_bignum_is_initialized(void);
fc_rounding_mode_t fc_bignum_current_rounding_mode(void);

#endif /* FC_BIGNUM_INTERNAL_H */
