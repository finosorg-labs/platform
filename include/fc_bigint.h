/**
 * @file fc_bigint.h
 * @brief High-precision integer API
 */

#ifndef FC_BIGINT_H
#define FC_BIGINT_H

#include "error.h"
#include "platform.h"

FC_BEGIN_DECLS

typedef struct fc_bigint fc_bigint_t;

FC_API fc_status_t fc_bigint_create(fc_bigint_t** value_out);
FC_API void fc_bigint_destroy(fc_bigint_t* value);
FC_API fc_status_t fc_bigint_copy(fc_bigint_t* dest, const fc_bigint_t* src);

FC_API fc_status_t fc_bigint_set_i64(fc_bigint_t* value, fc_int64_t input);
FC_API fc_status_t fc_bigint_set_u64(fc_bigint_t* value, fc_uint64_t input);
FC_API fc_status_t fc_bigint_set_str(fc_bigint_t* value, const char* input);
FC_API fc_status_t fc_bigint_get_i64(const fc_bigint_t* value, fc_int64_t* output);
FC_API fc_status_t fc_bigint_get_u64(const fc_bigint_t* value, fc_uint64_t* output);
FC_API fc_status_t fc_bigint_string_size(const fc_bigint_t* value, fc_size_t* size_out);
FC_API fc_status_t
fc_bigint_to_string(const fc_bigint_t* value, char* buffer, fc_size_t buffer_size);

FC_API fc_status_t
fc_bigint_add(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs);
FC_API fc_status_t
fc_bigint_sub(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs);
FC_API fc_status_t
fc_bigint_mul(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs);
FC_API fc_status_t
fc_bigint_div(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs);
FC_API fc_status_t
fc_bigint_mod(fc_bigint_t* result, const fc_bigint_t* lhs, const fc_bigint_t* rhs);
FC_API fc_status_t fc_bigint_cmp(const fc_bigint_t* lhs, const fc_bigint_t* rhs, int* cmp_out);

FC_END_DECLS

#endif /* FC_BIGINT_H */
