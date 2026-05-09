package platform

import "testing"

func TestBigIntBasic(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	lhs, err := ParseBigInt("12345678901234567890")
	if err != nil {
		t.Fatalf("ParseBigInt(lhs) failed: %v", err)
	}
	defer lhs.Destroy()

	rhs, err := ParseBigInt("10")
	if err != nil {
		t.Fatalf("ParseBigInt(rhs) failed: %v", err)
	}
	defer rhs.Destroy()

	result, err := NewBigInt()
	if err != nil {
		t.Fatalf("NewBigInt() failed: %v", err)
	}
	defer result.Destroy()

	if err := result.Add(lhs, rhs); err != nil {
		t.Fatalf("Add() failed: %v", err)
	}
	if got := result.String(); got != "12345678901234567900" {
		t.Fatalf("Add() = %q", got)
	}

	if err := result.Sub(lhs, rhs); err != nil {
		t.Fatalf("Sub() failed: %v", err)
	}
	if got := result.String(); got != "12345678901234567880" {
		t.Fatalf("Sub() = %q", got)
	}

	if err := result.Mul(rhs, rhs); err != nil {
		t.Fatalf("Mul() failed: %v", err)
	}
	if got := result.String(); got != "100" {
		t.Fatalf("Mul() = %q", got)
	}

	if err := result.Div(lhs, rhs); err != nil {
		t.Fatalf("Div() failed: %v", err)
	}
	if got := result.String(); got != "1234567890123456789" {
		t.Fatalf("Div() = %q", got)
	}

	if err := result.Mod(lhs, rhs); err != nil {
		t.Fatalf("Mod() failed: %v", err)
	}
	if got := result.String(); got != "0" {
		t.Fatalf("Mod() = %q", got)
	}
}

func TestBigIntErrors(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	value, err := NewBigInt()
	if err != nil {
		t.Fatalf("NewBigInt() failed: %v", err)
	}
	defer value.Destroy()

	if err := value.SetString("abc"); err == nil {
		t.Fatal("SetString() expected parse error")
	}

	zero, err := NewBigInt()
	if err != nil {
		t.Fatalf("NewBigInt(zero) failed: %v", err)
	}
	defer zero.Destroy()

	if err := zero.SetInt64(0); err != nil {
		t.Fatalf("zero.SetInt64() failed: %v", err)
	}
	if err := value.SetInt64(7); err != nil {
		t.Fatalf("value.SetInt64() failed: %v", err)
	}

	if err := value.Div(value, zero); err == nil {
		t.Fatal("Div() expected divide-by-zero error")
	}
}

func TestBigIntUint64Overflow(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	value, err := ParseBigInt("18446744073709551616")
	if err != nil {
		t.Fatalf("ParseBigInt() failed: %v", err)
	}
	defer value.Destroy()

	if _, err := value.Uint64(); err == nil {
		t.Fatal("Uint64() expected overflow error")
	}
}
