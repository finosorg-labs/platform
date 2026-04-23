//go:build !lib

// Source mode: compile C sources directly with CGO.
// Usage: go build (default)
// Requires: CGO_CFLAGS_ALLOW="-mavx.*|-msse.*|-mfma|-mavx512.*"
package platform

/*
#cgo CFLAGS: -I${SRCDIR}/include -I${SRCDIR}/src -O2 -Wall -std=c11 -D_POSIX_C_SOURCE=200112L
#cgo LDFLAGS: -lm

#include "platform.h"
#include "simd_detect.h"
#include "mem_aligned.h"
#include "error.h"

// Platform sources
#include "simd_detect.c"
#include "mem_aligned.c"
#include "error.c"
#include "fc_init.c"

// Platform-specific sources
#if defined(__linux__)
  #include "platform_linux.c"
#elif defined(__APPLE__)
  #include "platform_macos.c"
#elif defined(_WIN32)
  #include "platform_win.c"
#endif

// Forward declarations for fc_init/fc_cleanup (no public header)
int fc_init(void);
void fc_cleanup(void);
*/
import "C"
