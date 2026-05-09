package platform

import "testing"

func TestPrecisionDefaults(t *testing.T) {
	if err := Init(); err != nil {
		t.Fatalf("Init() failed: %v", err)
	}
	defer Cleanup()

	if err := SetDefaultPrecision(384); err != nil {
		t.Fatalf("SetDefaultPrecision() failed: %v", err)
	}
	bits, err := DefaultPrecision()
	if err != nil {
		t.Fatalf("DefaultPrecision() failed: %v", err)
	}
	if bits != 384 {
		t.Fatalf("DefaultPrecision() = %d", bits)
	}

	if err := SetRoundingMode(RoundTowardZero); err != nil {
		t.Fatalf("SetRoundingMode() failed: %v", err)
	}
	mode, err := DefaultRoundingMode()
	if err != nil {
		t.Fatalf("DefaultRoundingMode() failed: %v", err)
	}
	if mode != RoundTowardZero {
		t.Fatalf("DefaultRoundingMode() = %d", mode)
	}
}

func TestPrecisionRequiresValidBits(t *testing.T) {
	if err := SetDefaultPrecision(0); err == nil {
		t.Fatal("SetDefaultPrecision(0) expected error")
	}
}
