//go:build lib

// Prebuilt mode: link against prebuilt static library.
// Usage: go build -tags lib
// Requires: prebuilt libfinkit_platform_static.a in build/lib/
package platform

/*
#cgo CFLAGS: -I${SRCDIR}/include
#cgo linux,amd64   LDFLAGS: -L${SRCDIR}/build/lib -lfinkit_platform_static -lm
#cgo linux,arm64   LDFLAGS: -L${SRCDIR}/build/lib -lfinkit_platform_static -lm
#cgo darwin,amd64  LDFLAGS: -L${SRCDIR}/build/lib -lfinkit_platform_static -lm
#cgo darwin,arm64  LDFLAGS: -L${SRCDIR}/build/lib -lfinkit_platform_static -lm
#cgo windows,amd64 LDFLAGS: -L${SRCDIR}/build/lib -lfinkit_platform_static -lm

#include "platform.h"
#include "simd_detect.h"
#include "mem_aligned.h"
#include "error.h"

// Forward declarations for fc_init/fc_cleanup (no public header)
int fc_init(void);
void fc_cleanup(void);
*/
import "C"
