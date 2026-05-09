package platform

/*
#include <stdlib.h>
#include "fc_bigfloat.h"
*/
import "C"

import (
    "fmt"
    "runtime"
    "unsafe"
)

type BigFloat struct {
    ptr *C.fc_bigfloat_t
}

func (b *BigFloat) ensure() error {
    if b == nil || b.ptr == nil {
        return fmt.Errorf("bigfloat is nil")
    }
    return nil
}

func NewBigFloat() (*BigFloat, error) {
    var ptr *C.fc_bigfloat_t
    status := Status(C.fc_bigfloat_create(&ptr))
    if status != StatusOK {
        return nil, status.Error()
    }
    v := &BigFloat{ptr: ptr}
    runtime.SetFinalizer(v, func(b *BigFloat) { b.Destroy() })
    return v, nil
}

func NewBigFloatWithPrec(bits uint64) (*BigFloat, error) {
    var ptr *C.fc_bigfloat_t
    status := Status(C.fc_bigfloat_create_with_prec(&ptr, C.fc_uint64_t(bits)))
    if status != StatusOK {
        return nil, status.Error()
    }
    v := &BigFloat{ptr: ptr}
    runtime.SetFinalizer(v, func(b *BigFloat) { b.Destroy() })
    return v, nil
}

func ParseBigFloat(input string) (*BigFloat, error) {
    v, err := NewBigFloat()
    if err != nil {
        return nil, err
    }
    if err := v.SetString(input); err != nil {
        v.Destroy()
        return nil, err
    }
    return v, nil
}

func (b *BigFloat) Destroy() {
    if b == nil || b.ptr == nil {
        return
    }
    C.fc_bigfloat_destroy(b.ptr)
    b.ptr = nil
}

func (b *BigFloat) SetFloat64(input float64) error {
    if err := b.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_set_f64(b.ptr, C.fc_double_t(input))).Error()
}

func (b *BigFloat) SetInt64(input int64) error {
    if err := b.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_set_i64(b.ptr, C.fc_int64_t(input))).Error()
}

func (b *BigFloat) SetUint64(input uint64) error {
    if err := b.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_set_u64(b.ptr, C.fc_uint64_t(input))).Error()
}

func (b *BigFloat) SetString(input string) error {
    if err := b.ensure(); err != nil {
        return err
    }
    cstr := C.CString(input)
    defer C.free(unsafe.Pointer(cstr))
    return Status(C.fc_bigfloat_set_str(b.ptr, cstr)).Error()
}

func (b *BigFloat) SetBigInt(input *BigInt) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := input.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_set_bigint(b.ptr, input.ptr)).Error()
}

func (b *BigFloat) Float64() (float64, error) {
    if err := b.ensure(); err != nil {
        return 0, err
    }
    var out C.fc_double_t
    status := Status(C.fc_bigfloat_get_f64(b.ptr, &out))
    if status != StatusOK {
        return 0, status.Error()
    }
    return float64(out), nil
}

func (b *BigFloat) Precision() (uint64, error) {
    if err := b.ensure(); err != nil {
        return 0, err
    }
    var out C.fc_uint64_t
    status := Status(C.fc_bigfloat_get_prec(b.ptr, &out))
    if status != StatusOK {
        return 0, status.Error()
    }
    return uint64(out), nil
}

func (b *BigFloat) SetPrecision(bits uint64) error {
    if err := b.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_set_prec(b.ptr, C.fc_uint64_t(bits))).Error()
}

func (b *BigFloat) String() string {
    if err := b.ensure(); err != nil {
        return ""
    }
    var size C.fc_size_t
    if Status(C.fc_bigfloat_string_size(b.ptr, &size)) != StatusOK {
        return ""
    }
    buf := make([]byte, int(size))
    if Status(C.fc_bigfloat_to_string(b.ptr, (*C.char)(unsafe.Pointer(&buf[0])), size)) != StatusOK {
        return ""
    }
    return C.GoString((*C.char)(unsafe.Pointer(&buf[0])))
}

func (b *BigFloat) Add(lhs, rhs *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := lhs.ensure(); err != nil {
        return err
    }
    if err := rhs.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_add(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigFloat) Sub(lhs, rhs *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := lhs.ensure(); err != nil {
        return err
    }
    if err := rhs.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_sub(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigFloat) Mul(lhs, rhs *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := lhs.ensure(); err != nil {
        return err
    }
    if err := rhs.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_mul(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigFloat) Div(lhs, rhs *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := lhs.ensure(); err != nil {
        return err
    }
    if err := rhs.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_div(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigFloat) Neg(input *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := input.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_neg(b.ptr, input.ptr)).Error()
}

func (b *BigFloat) Abs(input *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := input.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_abs(b.ptr, input.ptr)).Error()
}

func (b *BigFloat) Sqrt(input *BigFloat) error {
    if err := b.ensure(); err != nil {
        return err
    }
    if err := input.ensure(); err != nil {
        return err
    }
    return Status(C.fc_bigfloat_sqrt(b.ptr, input.ptr)).Error()
}

func (b *BigFloat) Cmp(other *BigFloat) (int, error) {
    if err := b.ensure(); err != nil {
        return 0, err
    }
    if err := other.ensure(); err != nil {
        return 0, err
    }
    var cmp C.int
    status := Status(C.fc_bigfloat_cmp(b.ptr, other.ptr, &cmp))
    if status != StatusOK {
        return 0, status.Error()
    }
    return int(cmp), nil
}

func (b *BigFloat) GoString() string {
    return b.String()
}

