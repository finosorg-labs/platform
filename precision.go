package platform

/*
#include "fc_precision.h"
*/
import "C"

type RoundingMode int

const (
    RoundNearest RoundingMode = C.FC_RNDN
    RoundTowardZero RoundingMode = C.FC_RNDZ
    RoundUp RoundingMode = C.FC_RNDU
    RoundDown RoundingMode = C.FC_RNDD
    RoundAwayFromZero RoundingMode = C.FC_RNDA
)

func SetDefaultPrecision(bits uint64) error {
    return Status(C.fc_precision_set_default_bits(C.fc_uint64_t(bits))).Error()
}

func DefaultPrecision() (uint64, error) {
    var out C.fc_uint64_t
    status := Status(C.fc_precision_get_default_bits(&out))
    if status != StatusOK {
        return 0, status.Error()
    }
    return uint64(out), nil
}

func SetRoundingMode(mode RoundingMode) error {
    return Status(C.fc_precision_set_rounding_mode(C.fc_rounding_mode_t(mode))).Error()
}

func DefaultRoundingMode() (RoundingMode, error) {
    var out C.fc_rounding_mode_t
    status := Status(C.fc_precision_get_rounding_mode(&out))
    if status != StatusOK {
        return 0, status.Error()
    }
    return RoundingMode(out), nil
}
