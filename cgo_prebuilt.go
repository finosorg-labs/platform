//go:build lib

// Prebuilt mode: link against prebuilt static library.
// Usage: go build -tags lib
// Requires: prebuilt libfinkit_platform_static.a in build/linux_amd64/
package platform

/*
#cgo CFLAGS: -I${SRCDIR}/include
#cgo linux,amd64   LDFLAGS: -L${SRCDIR}/build/linux_amd64 -lfinkit_platform_static ${SRCDIR}/third_party/mpfr/src/.libs/libmpfr.a ${SRCDIR}/third_party/gmp/.libs/libgmp.a -lm
#cgo linux,arm64   LDFLAGS: -L${SRCDIR}/build/linux_arm64 -lfinkit_platform_static ${SRCDIR}/third_party/mpfr/src/.libs/libmpfr.a ${SRCDIR}/third_party/gmp/.libs/libgmp.a -lm
#cgo darwin,amd64  LDFLAGS: -L${SRCDIR}/build/darwin_amd64 -lfinkit_platform_static ${SRCDIR}/third_party/mpfr/src/.libs/libmpfr.a ${SRCDIR}/third_party/gmp/.libs/libgmp.a -lm
#cgo darwin,arm64  LDFLAGS: -L${SRCDIR}/build/darwin_arm64 -lfinkit_platform_static ${SRCDIR}/third_party/mpfr/src/.libs/libmpfr.a ${SRCDIR}/third_party/gmp/.libs/libgmp.a -lm
#cgo windows,amd64 LDFLAGS: -L${SRCDIR}/build/windows_amd64 -lfinkit_platform_static ${SRCDIR}/third_party/mpfr/src/.libs/libmpfr.a ${SRCDIR}/third_party/gmp/.libs/libgmp.a -lm

#include "platform.h"
#include "simd_detect.h"
#include "mem_aligned.h"
#include "error.h"
#include "fc_bigint.h"
#include "fc_bigfloat.h"
#include "fc_precision.h"

// Forward declarations for fc_init/fc_cleanup (no public header)
int fc_init(void);
void fc_cleanup(void);
*/
import "C"
