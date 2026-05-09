package platform

import (
	"math"
	"testing"
)

func TestBigFloatBasic(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	lhs, err := ParseBigFloat("1.5")
	if err != nil {
		t.Fatalf("ParseBigFloat(lhs) failed: %v", err)
	}
	defer lhs.Destroy()

	rhs, err := ParseBigFloat("2.0")
	if err != nil {
		t.Fatalf("ParseBigFloat(rhs) failed: %v", err)
	}
	defer rhs.Destroy()

	result, err := NewBigFloat()
	if err != nil {
		t.Fatalf("NewBigFloat() failed: %v", err)
	}
	defer result.Destroy()

	if err := result.Add(lhs, rhs); err != nil {
		t.Fatalf("Add() failed: %v", err)
	}
	if got, err := result.Float64(); err != nil || math.Abs(got-3.5) > 1e-12 {
		t.Fatalf("Add() = %v, %v", got, err)
	}

	if err := result.Mul(lhs, rhs); err != nil {
		t.Fatalf("Mul() failed: %v", err)
	}
	if got, err := result.Float64(); err != nil || math.Abs(got-3.0) > 1e-12 {
		t.Fatalf("Mul() = %v, %v", got, err)
	}

	if err := result.Sqrt(rhs); err != nil {
		t.Fatalf("Sqrt() failed: %v", err)
	}
	if got, err := result.Float64(); err != nil || math.Abs(got-math.Sqrt(2.0)) > 1e-12 {
		t.Fatalf("Sqrt() = %v, %v", got, err)
	}
}

func TestBigFloatPrecisionAndBigIntConversion(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	value, err := NewBigFloatWithPrec(512)
	if err != nil {
		t.Fatalf("NewBigFloatWithPrec() failed: %v", err)
	}
	defer value.Destroy()

	if got, err := value.Precision(); err != nil || got != 512 {
		t.Fatalf("Precision() = %d, %v", got, err)
	}

	integer, err := ParseBigInt("42")
	if err != nil {
		t.Fatalf("ParseBigInt() failed: %v", err)
	}
	defer integer.Destroy()

	if err := value.SetBigInt(integer); err != nil {
		t.Fatalf("SetBigInt() failed: %v", err)
	}
	if got, err := value.Float64(); err != nil || got != 42 {
		t.Fatalf("Float64() = %v, %v", got, err)
	}
}

func TestBigFloatErrors(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	value, err := NewBigFloat()
	if err != nil {
		t.Fatalf("NewBigFloat() failed: %v", err)
	}
	defer value.Destroy()

	if err := value.SetString("abc"); err == nil {
		t.Fatal("SetString() expected parse error")
	}

	neg, err := ParseBigFloat("-1")
	if err != nil {
		t.Fatalf("ParseBigFloat(-1) failed: %v", err)
	}
	defer neg.Destroy()

	if err := value.Sqrt(neg); err == nil {
		t.Fatal("Sqrt() expected invalid argument error")
	}

	zero, err := ParseBigFloat("0")
	if err != nil {
		t.Fatalf("ParseBigFloat(0) failed: %v", err)
	}
	defer zero.Destroy()

	one, err := ParseBigFloat("1")
	if err != nil {
		t.Fatalf("ParseBigFloat(1) failed: %v", err)
	}
	defer one.Destroy()

	if err := value.Div(one, zero); err == nil {
		t.Fatal("Div() expected divide-by-zero error")
	}
}
