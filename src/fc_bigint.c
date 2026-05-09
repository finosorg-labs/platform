#include "fc_bigint.h"

#include "fc_bignum_internal.h"

#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static fc_status_t fc_bigint_require_ready(const fc_bigint_t* value) {
    if (!fc_bignum_is_initialized()) {
        return FC_ERR_NOT_INITIALIZED;
    }
    if (value == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    return FC_OK;
}

FC_API fc_status_t fc_bigint_create(fc_bigint_t** value_out) {
    fc_bigint_impl_t* value;

    if (value_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    if (!fc_bignum_is_initialized()) {
        return FC_ERR_NOT_INITIALIZED;
    }

    value = (fc_bigint_impl_t*) malloc(sizeof(*value));
    if (value == FC_NULL) {
        return FC_ERR_OUT_OF_MEMORY;
    }

    mpz_init(value->value);
    *value_out = (fc_bigint_t*) value;
    return FC_OK;
}

FC_API void fc_bigint_destroy(fc_bigint_t* value) {
    fc_bigint_impl_t* impl = (fc_bigint_impl_t*) value;
    if (impl == FC_NULL) {
        return;
    }

    mpz_clear(impl->value);
    free(impl);
}

FC_API fc_status_t fc_bigint_copy(fc_bigint_t* dest, const fc_bigint_t* src) {
    fc_status_t status = fc_bigint_require_ready(dest);
    if (status != FC_OK) {
        return status;
    }
    status = fc_bigint_require_ready(src);
    if (status != FC_OK) {
        return status;
    }

    mpz_set(((fc_bigint_impl_t*) dest)->value, ((const fc_bigint_impl_t*) src)->value);
    return FC_OK;
}

FC_API fc_status_t fc_bigint_set_i64(fc_bigint_t* value, fc_int64_t input) {
    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }

    if (input >= LONG_MIN && input <= LONG_MAX) {
        mpz_set_si(((fc_bigint_impl_t*) value)->value, (long) input);
        return FC_OK;
    }

    mpz_import(((fc_bigint_impl_t*) value)->value, 1, -1, sizeof(input), 0, 0, &input);
    if (input < 0) {
        mpz_neg(((fc_bigint_impl_t*) value)->value, ((fc_bigint_impl_t*) value)->value);
    }
    return FC_OK;
}

FC_API fc_status_t fc_bigint_set_u64(fc_bigint_t* value, fc_uint64_t input) {
    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }

    if (input <= ULONG_MAX) {
        mpz_set_ui(((fc_bigint_impl_t*) value)->value, (unsigned long) input);
        return FC_OK;
    }

    mpz_import(((fc_bigint_impl_t*) value)->value, 1, -1, sizeof(input), 0, 0, &input);
    return FC_OK;
}

FC_API fc_status_t fc_bigint_set_str(fc_bigint_t* value, const char* input) {
    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (input == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    if (mpz_set_str(((fc_bigint_impl_t*) value)->value, input, 10) != 0) {
        return FC_ERR_PARSE;
    }
    return FC_OK;
}

FC_API fc_status_t fc_bigint_get_i64(const fc_bigint_t* value, fc_int64_t* output) {
    const fc_bigint_impl_t* impl = (const fc_bigint_impl_t*) value;
    uint64_t magnitude           = 0;
    size_t count                 = 0;

    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (output == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    if (mpz_sgn(impl->value) >= 0) {
        if (!mpz_fits_ulong_p(impl->value) && mpz_sizeinbase(impl->value, 2) > 64) {
            return FC_ERR_OVERFLOW;
        }
        mpz_export(&magnitude, &count, -1, sizeof(magnitude), 0, 0, impl->value);
        if (count == 0) {
            *output = 0;
            return FC_OK;
        }
        if (magnitude > INT64_MAX) {
            return FC_ERR_OVERFLOW;
        }
        *output = (fc_int64_t) magnitude;
        return FC_OK;
    }

    {
        mpz_t temp;
        mpz_init(temp);
        mpz_abs(temp, impl->value);
        if (mpz_sizeinbase(temp, 2) > 64) {
            mpz_clear(temp);
            return FC_ERR_OVERFLOW;
        }
        mpz_export(&magnitude, &count, -1, sizeof(magnitude), 0, 0, temp);
        mpz_clear(temp);
    }

    if (count == 0) {
        *output = 0;
        return FC_OK;
    }
    if (magnitude > ((uint64_t) INT64_MAX + 1U)) {
        return FC_ERR_OVERFLOW;
    }
    if (magnitude == ((uint64_t) INT64_MAX + 1U)) {
        *output = INT64_MIN;
        return FC_OK;
    }
    *output = -(fc_int64_t) magnitude;
    return FC_OK;
}

FC_API fc_status_t fc_bigint_get_u64(const fc_bigint_t* value, fc_uint64_t* output) {
    const fc_bigint_impl_t* impl = (const fc_bigint_impl_t*) value;
    size_t count                 = 0;
    uint64_t magnitude           = 0;

    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (output == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    if (mpz_sgn(impl->value) < 0) {
        return FC_ERR_OVERFLOW;
    }
    if (mpz_sizeinbase(impl->value, 2) > 64) {
        return FC_ERR_OVERFLOW;
    }

    mpz_export(&magnitude, &count, -1, sizeof(magnitude), 0, 0, impl->value);
    *output = count == 0 ? 0 : (fc_uint64_t) magnitude;
    return FC_OK;
}

FC_API fc_status_t fc_bigint_string_size(const fc_bigint_t* value, fc_size_t* size_out) {
    const fc_bigint_impl_t* impl = (const fc_bigint_impl_t*) value;
    size_t digits;

    fc_status_t status = fc_bigint_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (size_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    digits = mpz_sizeinbase(impl->value, 10);
    if (mpz_sgn(impl->value) < 0) {
        digits++;
    }
    *size_out = digits + 1;
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_to_string(const fc_bigint_t* value, char* buffer, fc_size_t buffer_size) {
    fc_size_t required_size;
    fc_status_t status = fc_bigint_string_size(value, &required_size);
    if (status != FC_OK) {
        return status;
    }
    if (buffer == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    if (buffer_size < required_size) {
        return FC_ERR_BUFFER_TOO_SMALL;
    }

    if (mpz_get_str(buffer, 10, ((const fc_bigint_impl_t*) value)->value) == FC_NULL) {
        return FC_ERR_OUT_OF_MEMORY;
    }
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_add(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs) {
    fc_status_t status = fc_bigint_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(lhs)) != FC_OK ||
        (status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpz_add(
        ((fc_bigint_impl_t*) result)->value,
        ((const fc_bigint_impl_t*) lhs)->value,
        ((const fc_bigint_impl_t*) rhs)->value
    );
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_sub(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs) {
    fc_status_t status = fc_bigint_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(lhs)) != FC_OK ||
        (status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpz_sub(
        ((fc_bigint_impl_t*) result)->value,
        ((const fc_bigint_impl_t*) lhs)->value,
        ((const fc_bigint_impl_t*) rhs)->value
    );
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_mul(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs) {
    fc_status_t status = fc_bigint_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(lhs)) != FC_OK ||
        (status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpz_mul(
        ((fc_bigint_impl_t*) result)->value,
        ((const fc_bigint_impl_t*) lhs)->value,
        ((const fc_bigint_impl_t*) rhs)->value
    );
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_div(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs) {
    fc_status_t status = fc_bigint_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(lhs)) != FC_OK ||
        (status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    if (mpz_sgn(((const fc_bigint_impl_t*) rhs)->value) == 0) {
        return FC_ERR_DIVIDE_BY_ZERO;
    }
    mpz_tdiv_q(
        ((fc_bigint_impl_t*) result)->value,
        ((const fc_bigint_impl_t*) lhs)->value,
        ((const fc_bigint_impl_t*) rhs)->value
    );
    return FC_OK;
}

FC_API fc_status_t
fc_bigint_mod(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs) {
    fc_status_t status = fc_bigint_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(lhs)) != FC_OK ||
        (status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    if (mpz_sgn(((const fc_bigint_impl_t*) rhs)->value) == 0) {
        return FC_ERR_DIVIDE_BY_ZERO;
    }
    mpz_mod(
        ((fc_bigint_impl_t*) result)->value,
        ((const fc_bigint_impl_t*) lhs)->value,
        ((const fc_bigint_impl_t*) rhs)->value
    );
    return FC_OK;
}

FC_API fc_status_t fc_bigint_cmp(const fc_bigint_t* lhs, const fc_bigint_t* rhs, int* cmp_out) {
    fc_status_t status = fc_bigint_require_ready(lhs);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigint_require_ready(rhs)) != FC_OK) {
        return status;
    }
    if (cmp_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    *cmp_out =
        mpz_cmp(((const fc_bigint_impl_t*) lhs)->value, ((const fc_bigint_impl_t*) rhs)->value);
    return FC_OK;
}
