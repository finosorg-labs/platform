#include "fc_bigfloat.h"

#include "fc_bignum_internal.h"

#include <gmp.h>
#include <math.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mpfr_rnd_t fc_bigfloat_rounding_mode(void) {
    switch (fc_bignum_current_rounding_mode()) {
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

static fc_status_t fc_bigfloat_require_ready(const fc_bigfloat_t* value) {
    if (!fc_bignum_is_initialized()) {
        return FC_ERR_NOT_INITIALIZED;
    }
    if (value == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    return FC_OK;
}

static fc_status_t fc_bigfloat_check_flags(void) {
    if (mpfr_divby0_p()) {
        return FC_ERR_DIVIDE_BY_ZERO;
    }
    if (mpfr_overflow_p()) {
        return FC_ERR_OVERFLOW;
    }
    if (mpfr_underflow_p()) {
        return FC_ERR_UNDERFLOW;
    }
    return FC_OK;
}

static fc_status_t fc_bigfloat_format_regular(
    const mpfr_t value,
    char* buffer,
    fc_size_t buffer_size
) {
    mpfr_exp_t exponent;
    char* digits;
    char* magnitude;
    size_t digits_len;
    size_t sign_len = 0;
    int written;

    digits = mpfr_get_str(FC_NULL, &exponent, 10, 0, value, fc_bigfloat_rounding_mode());
    if (digits == FC_NULL) {
        return FC_ERR_OUT_OF_MEMORY;
    }

    magnitude = digits;
    if (digits[0] == '-') {
        sign_len  = 1;
        magnitude = digits + 1;
    }
    digits_len = strlen(magnitude);

    if (digits_len == 0) {
        mpfr_free_str(digits);
        return FC_ERR_INTERNAL;
    }

    if (digits_len == 1) {
        written = snprintf(
            buffer,
            buffer_size,
            "%.*s%se%ld",
            (int) sign_len,
            digits,
            magnitude,
            (long) (exponent - 1)
        );
    } else {
        written = snprintf(
            buffer,
            buffer_size,
            "%.*s%c.%se%ld",
            (int) sign_len,
            digits,
            magnitude[0],
            magnitude + 1,
            (long) (exponent - 1)
        );
    }

    mpfr_free_str(digits);
    if (written < 0) {
        return FC_ERR_INTERNAL;
    }
    if ((fc_size_t) written + 1 > buffer_size) {
        return FC_ERR_BUFFER_TOO_SMALL;
    }
    return FC_OK;
}

FC_API fc_status_t fc_bigfloat_create(fc_bigfloat_t** value_out) {
    fc_uint64_t bits = 0;
    fc_status_t status;

    if (value_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    if (!fc_bignum_is_initialized()) {
        return FC_ERR_NOT_INITIALIZED;
    }

    status = fc_precision_get_default_bits(&bits);
    if (status != FC_OK) {
        return status;
    }
    return fc_bigfloat_create_with_prec(value_out, bits);
}

FC_API fc_status_t fc_bigfloat_create_with_prec(fc_bigfloat_t** value_out, fc_uint64_t bits) {
    fc_bigfloat_impl_t* value;

    if (value_out == FC_NULL || bits == 0 || bits > (fc_uint64_t) MPFR_PREC_MAX) {
        return FC_ERR_INVALID_ARG;
    }
    if (!fc_bignum_is_initialized()) {
        return FC_ERR_NOT_INITIALIZED;
    }

    value = (fc_bigfloat_impl_t*) malloc(sizeof(*value));
    if (value == FC_NULL) {
        return FC_ERR_OUT_OF_MEMORY;
    }

    mpfr_init2(value->value, (mpfr_prec_t) bits);
    mpfr_set_zero(value->value, 0);
    *value_out = (fc_bigfloat_t*) value;
    return FC_OK;
}

FC_API void fc_bigfloat_destroy(fc_bigfloat_t* value) {
    fc_bigfloat_impl_t* impl = (fc_bigfloat_impl_t*) value;
    if (impl == FC_NULL) {
        return;
    }

    mpfr_clear(impl->value);
    free(impl);
}

FC_API fc_status_t fc_bigfloat_copy(fc_bigfloat_t* dest, const fc_bigfloat_t* src) {
    fc_status_t status = fc_bigfloat_require_ready(dest);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(src)) != FC_OK) {
        return status;
    }

    mpfr_set_prec(
        ((fc_bigfloat_impl_t*) dest)->value, mpfr_get_prec(((const fc_bigfloat_impl_t*) src)->value)
    );
    mpfr_clear_flags();
    mpfr_set(
        ((fc_bigfloat_impl_t*) dest)->value,
        ((const fc_bigfloat_impl_t*) src)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_get_prec(const fc_bigfloat_t* value, fc_uint64_t* bits_out) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (bits_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    *bits_out = (fc_uint64_t) mpfr_get_prec(((const fc_bigfloat_impl_t*) value)->value);
    return FC_OK;
}

FC_API fc_status_t fc_bigfloat_set_prec(fc_bigfloat_t* value, fc_uint64_t bits) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (bits == 0 || bits > (fc_uint64_t) MPFR_PREC_MAX) {
        return FC_ERR_INVALID_ARG;
    }

    mpfr_set_prec(((fc_bigfloat_impl_t*) value)->value, (mpfr_prec_t) bits);
    return FC_OK;
}

FC_API fc_status_t fc_bigfloat_set_f64(fc_bigfloat_t* value, fc_double_t input) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (!isfinite(input)) {
        return FC_ERR_INVALID_ARG;
    }

    mpfr_clear_flags();
    mpfr_set_d(((fc_bigfloat_impl_t*) value)->value, input, fc_bigfloat_rounding_mode());
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_set_i64(fc_bigfloat_t* value, fc_int64_t input) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }

    mpfr_clear_flags();
    if (input >= LONG_MIN && input <= LONG_MAX) {
        mpfr_set_si(
            ((fc_bigfloat_impl_t*) value)->value, (long) input, fc_bigfloat_rounding_mode()
        );
    } else {
        fc_bigint_impl_t temp;
        mpz_init(temp.value);
        if (input >= 0) {
            mpz_import(temp.value, 1, -1, sizeof(input), 0, 0, &input);
        } else {
            uint64_t magnitude = (uint64_t) (-(input + 1)) + 1U;
            mpz_import(temp.value, 1, -1, sizeof(magnitude), 0, 0, &magnitude);
            mpz_neg(temp.value, temp.value);
        }
        mpfr_set_z(((fc_bigfloat_impl_t*) value)->value, temp.value, fc_bigfloat_rounding_mode());
        mpz_clear(temp.value);
    }
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_set_u64(fc_bigfloat_t* value, fc_uint64_t input) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }

    mpfr_clear_flags();
    if (input <= ULONG_MAX) {
        mpfr_set_ui(
            ((fc_bigfloat_impl_t*) value)->value, (unsigned long) input, fc_bigfloat_rounding_mode()
        );
    } else {
        fc_bigint_impl_t temp;
        mpz_init(temp.value);
        mpz_import(temp.value, 1, -1, sizeof(input), 0, 0, &input);
        mpfr_set_z(((fc_bigfloat_impl_t*) value)->value, temp.value, fc_bigfloat_rounding_mode());
        mpz_clear(temp.value);
    }
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_set_str(fc_bigfloat_t* value, const char* input) {
    char* end          = FC_NULL;
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (input == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    mpfr_clear_flags();
    if (mpfr_strtofr(
            ((fc_bigfloat_impl_t*) value)->value, input, &end, 10, fc_bigfloat_rounding_mode()
        ) == 0) {
        if (end == input || (end != FC_NULL && *end != '\0')) {
            return FC_ERR_PARSE;
        }
    } else if (end == input || (end != FC_NULL && *end != '\0')) {
        return FC_ERR_PARSE;
    }

    if (mpfr_nan_p(((fc_bigfloat_impl_t*) value)->value) ||
        mpfr_inf_p(((fc_bigfloat_impl_t*) value)->value)) {
        return FC_ERR_INVALID_ARG;
    }
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_set_bigint(fc_bigfloat_t* value, const fc_bigint_t* input) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (!fc_bignum_is_initialized() || input == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    mpfr_clear_flags();
    mpfr_set_z(
        ((fc_bigfloat_impl_t*) value)->value,
        ((const fc_bigint_impl_t*) input)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_get_f64(const fc_bigfloat_t* value, fc_double_t* output) {
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (output == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    mpfr_clear_flags();
    *output = mpfr_get_d(((const fc_bigfloat_impl_t*) value)->value, fc_bigfloat_rounding_mode());
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_string_size(const fc_bigfloat_t* value, fc_size_t* size_out) {
    char stack_buffer[2048];
    fc_status_t status = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (size_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }

    status = fc_bigfloat_to_string(value, stack_buffer, sizeof(stack_buffer));
    if (status == FC_ERR_BUFFER_TOO_SMALL) {
        return FC_ERR_OUT_OF_MEMORY;
    }
    if (status != FC_OK) {
        return status;
    }

    *size_out = strlen(stack_buffer) + 1;
    return FC_OK;
}

FC_API fc_status_t
fc_bigfloat_to_string(const fc_bigfloat_t* value, char* buffer, fc_size_t buffer_size) {
    const fc_bigfloat_impl_t* impl = (const fc_bigfloat_impl_t*) value;
    fc_status_t status             = fc_bigfloat_require_ready(value);
    if (status != FC_OK) {
        return status;
    }
    if (buffer == FC_NULL || buffer_size == 0) {
        return FC_ERR_INVALID_ARG;
    }

    if (mpfr_nan_p(impl->value)) {
        if (buffer_size < 4) {
            return FC_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(buffer, "nan", 4);
        return FC_OK;
    }
    if (mpfr_inf_p(impl->value)) {
        const char* text = mpfr_sgn(impl->value) < 0 ? "-inf" : "inf";
        size_t needed    = strlen(text) + 1;
        if (buffer_size < needed) {
            return FC_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(buffer, text, needed);
        return FC_OK;
    }
    if (mpfr_zero_p(impl->value)) {
        const char* text = mpfr_signbit(impl->value) ? "-0" : "0";
        size_t needed    = strlen(text) + 1;
        if (buffer_size < needed) {
            return FC_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(buffer, text, needed);
        return FC_OK;
    }

    return fc_bigfloat_format_regular(impl->value, buffer, buffer_size);
}

FC_API fc_status_t
fc_bigfloat_add(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(lhs)) != FC_OK ||
        (status = fc_bigfloat_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpfr_clear_flags();
    mpfr_add(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) lhs)->value,
        ((const fc_bigfloat_impl_t*) rhs)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t
fc_bigfloat_sub(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(lhs)) != FC_OK ||
        (status = fc_bigfloat_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpfr_clear_flags();
    mpfr_sub(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) lhs)->value,
        ((const fc_bigfloat_impl_t*) rhs)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t
fc_bigfloat_mul(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(lhs)) != FC_OK ||
        (status = fc_bigfloat_require_ready(rhs)) != FC_OK) {
        return status;
    }
    mpfr_clear_flags();
    mpfr_mul(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) lhs)->value,
        ((const fc_bigfloat_impl_t*) rhs)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t
fc_bigfloat_div(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(lhs)) != FC_OK ||
        (status = fc_bigfloat_require_ready(rhs)) != FC_OK) {
        return status;
    }
    if (mpfr_zero_p(((const fc_bigfloat_impl_t*) rhs)->value)) {
        return FC_ERR_DIVIDE_BY_ZERO;
    }
    mpfr_clear_flags();
    mpfr_div(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) lhs)->value,
        ((const fc_bigfloat_impl_t*) rhs)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_neg(fc_bigfloat_t* result, const fc_bigfloat_t* input) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(input)) != FC_OK) {
        return status;
    }
    mpfr_clear_flags();
    mpfr_neg(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) input)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_abs(fc_bigfloat_t* result, const fc_bigfloat_t* input) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(input)) != FC_OK) {
        return status;
    }
    mpfr_clear_flags();
    mpfr_abs(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) input)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t fc_bigfloat_sqrt(fc_bigfloat_t* result, const fc_bigfloat_t* input) {
    fc_status_t status = fc_bigfloat_require_ready(result);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(input)) != FC_OK) {
        return status;
    }
    if (mpfr_sgn(((const fc_bigfloat_impl_t*) input)->value) < 0) {
        return FC_ERR_INVALID_ARG;
    }
    mpfr_clear_flags();
    mpfr_sqrt(
        ((fc_bigfloat_impl_t*) result)->value,
        ((const fc_bigfloat_impl_t*) input)->value,
        fc_bigfloat_rounding_mode()
    );
    return fc_bigfloat_check_flags();
}

FC_API fc_status_t
fc_bigfloat_cmp(const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs, int* cmp_out) {
    fc_status_t status = fc_bigfloat_require_ready(lhs);
    if (status != FC_OK) {
        return status;
    }
    if ((status = fc_bigfloat_require_ready(rhs)) != FC_OK) {
        return status;
    }
    if (cmp_out == FC_NULL) {
        return FC_ERR_INVALID_ARG;
    }
    *cmp_out = mpfr_cmp(
        ((const fc_bigfloat_impl_t*) lhs)->value, ((const fc_bigfloat_impl_t*) rhs)->value
    );
    return FC_OK;
}
