/**
 * @file fc_bigfloat.h
 * @brief High-precision floating-point API
 */

#ifndef FC_BIGFLOAT_H
#define FC_BIGFLOAT_H

#include "error.h"
#include "fc_bigint.h"
#include "fc_precision.h"
#include "platform.h"

FC_BEGIN_DECLS

typedef struct fc_bigfloat fc_bigfloat_t;

FC_API fc_status_t fc_bigfloat_create(fc_bigfloat_t** value_out);
FC_API fc_status_t fc_bigfloat_create_with_prec(fc_bigfloat_t** value_out, fc_uint64_t bits);
FC_API void fc_bigfloat_destroy(fc_bigfloat_t* value);
FC_API fc_status_t fc_bigfloat_copy(fc_bigfloat_t* dest, const fc_bigfloat_t* src);

FC_API fc_status_t fc_bigfloat_get_prec(const fc_bigfloat_t* value, fc_uint64_t* bits_out);
FC_API fc_status_t fc_bigfloat_set_prec(fc_bigfloat_t* value, fc_uint64_t bits);

FC_API fc_status_t fc_bigfloat_set_f64(fc_bigfloat_t* value, fc_double_t input);
FC_API fc_status_t fc_bigfloat_set_i64(fc_bigfloat_t* value, fc_int64_t input);
FC_API fc_status_t fc_bigfloat_set_u64(fc_bigfloat_t* value, fc_uint64_t input);
FC_API fc_status_t fc_bigfloat_set_str(fc_bigfloat_t* value, const char* input);
FC_API fc_status_t fc_bigfloat_set_bigint(fc_bigfloat_t* value, const fc_bigint_t* input);
FC_API fc_status_t fc_bigfloat_get_f64(const fc_bigfloat_t* value, fc_double_t* output);
FC_API fc_status_t fc_bigfloat_string_size(const fc_bigfloat_t* value, fc_size_t* size_out);
FC_API fc_status_t
fc_bigfloat_to_string(const fc_bigfloat_t* value, char* buffer, fc_size_t buffer_size);

FC_API fc_status_t
fc_bigfloat_add(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs);
FC_API fc_status_t
fc_bigfloat_sub(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs);
FC_API fc_status_t
fc_bigfloat_mul(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs);
FC_API fc_status_t
fc_bigfloat_div(fc_bigfloat_t* result, const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs);
FC_API fc_status_t fc_bigfloat_neg(fc_bigfloat_t* result, const fc_bigfloat_t* input);
FC_API fc_status_t fc_bigfloat_abs(fc_bigfloat_t* result, const fc_bigfloat_t* input);
FC_API fc_status_t fc_bigfloat_sqrt(fc_bigfloat_t* result, const fc_bigfloat_t* input);
FC_API fc_status_t
fc_bigfloat_cmp(const fc_bigfloat_t* lhs, const fc_bigfloat_t* rhs, int* cmp_out);

FC_END_DECLS

#endif /* FC_BIGFLOAT_H */
