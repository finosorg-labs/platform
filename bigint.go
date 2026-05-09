package platform

/*
#include <stdlib.h>
#include "fc_bigint.h"
*/
import "C"

import (
	"fmt"
	"runtime"
	"unsafe"
)

type BigInt struct {
	ptr *C.fc_bigint_t
}

func (b *BigInt) ensure() error {
	if b == nil || b.ptr == nil {
		return fmt.Errorf("bigint is nil")
	}
	return nil
}

func NewBigInt() (*BigInt, error) {
	var ptr *C.fc_bigint_t
	status := Status(C.fc_bigint_create(&ptr))
	if status != StatusOK {
		return nil, status.Error()
	}

	v := &BigInt{ptr: ptr}
	runtime.SetFinalizer(v, func(b *BigInt) { b.Destroy() })
	return v, nil
}

func ParseBigInt(input string) (*BigInt, error) {
	v, err := NewBigInt()
	if err != nil {
		return nil, err
	}
	if err := v.SetString(input); err != nil {
		v.Destroy()
		return nil, err
	}
	return v, nil
}

func (b *BigInt) Destroy() {
	if b == nil || b.ptr == nil {
		return
	}
	C.fc_bigint_destroy(b.ptr)
	b.ptr = nil
}

func (b *BigInt) SetInt64(input int64) error {
	if b == nil || b.ptr == nil {
		return fmt.Errorf("bigint is nil")
	}
	return Status(C.fc_bigint_set_i64(b.ptr, C.fc_int64_t(input))).Error()
}

func (b *BigInt) SetUint64(input uint64) error {
	if b == nil || b.ptr == nil {
		return fmt.Errorf("bigint is nil")
	}
	return Status(C.fc_bigint_set_u64(b.ptr, C.fc_uint64_t(input))).Error()
}

func (b *BigInt) SetString(input string) error {
	if b == nil || b.ptr == nil {
		return fmt.Errorf("bigint is nil")
	}
	cstr := C.CString(input)
	defer C.free(unsafe.Pointer(cstr))
	return Status(C.fc_bigint_set_str(b.ptr, cstr)).Error()
}

func (b *BigInt) Int64() (int64, error) {
	if b == nil || b.ptr == nil {
		return 0, fmt.Errorf("bigint is nil")
	}
	var out C.fc_int64_t
	status := Status(C.fc_bigint_get_i64(b.ptr, &out))
	if status != StatusOK {
		return 0, status.Error()
	}
	return int64(out), nil
}

func (b *BigInt) Uint64() (uint64, error) {
	if b == nil || b.ptr == nil {
		return 0, fmt.Errorf("bigint is nil")
	}
	var out C.fc_uint64_t
	status := Status(C.fc_bigint_get_u64(b.ptr, &out))
	if status != StatusOK {
		return 0, status.Error()
	}
	return uint64(out), nil
}

func (b *BigInt) String() string {
	if b == nil || b.ptr == nil {
		return ""
	}
	var size C.fc_size_t
	if Status(C.fc_bigint_string_size(b.ptr, &size)) != StatusOK {
		return ""
	}
	buf := make([]byte, int(size))
	if Status(C.fc_bigint_to_string(b.ptr, (*C.char)(unsafe.Pointer(&buf[0])), size)) != StatusOK {
		return ""
	}
	return C.GoString((*C.char)(unsafe.Pointer(&buf[0])))
}

func (b *BigInt) Add(lhs, rhs *BigInt) error {
	if err := b.ensure(); err != nil {
		return err
	}
	if err := lhs.ensure(); err != nil {
		return err
	}
	if err := rhs.ensure(); err != nil {
		return err
	}
	return Status(C.fc_bigint_add(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigInt) Sub(lhs, rhs *BigInt) error {
	if err := b.ensure(); err != nil {
		return err
	}
	if err := lhs.ensure(); err != nil {
		return err
	}
	if err := rhs.ensure(); err != nil {
		return err
	}
	return Status(C.fc_bigint_sub(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigInt) Mul(lhs, rhs *BigInt) error {
	if err := b.ensure(); err != nil {
		return err
	}
	if err := lhs.ensure(); err != nil {
		return err
	}
	if err := rhs.ensure(); err != nil {
		return err
	}
	return Status(C.fc_bigint_mul(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigInt) Div(lhs, rhs *BigInt) error {
	if err := b.ensure(); err != nil {
		return err
	}
	if err := lhs.ensure(); err != nil {
		return err
	}
	if err := rhs.ensure(); err != nil {
		return err
	}
	return Status(C.fc_bigint_div(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigInt) Mod(lhs, rhs *BigInt) error {
	if err := b.ensure(); err != nil {
		return err
	}
	if err := lhs.ensure(); err != nil {
		return err
	}
	if err := rhs.ensure(); err != nil {
		return err
	}
	return Status(C.fc_bigint_mod(b.ptr, lhs.ptr, rhs.ptr)).Error()
}

func (b *BigInt) Cmp(other *BigInt) (int, error) {
	if err := b.ensure(); err != nil {
		return 0, err
	}
	if err := other.ensure(); err != nil {
		return 0, err
	}
	var cmp C.int
	status := Status(C.fc_bigint_cmp(b.ptr, other.ptr, &cmp))
	if status != StatusOK {
		return 0, status.Error()
	}
	return int(cmp), nil
}
