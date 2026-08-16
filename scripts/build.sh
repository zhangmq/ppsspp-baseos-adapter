#!/bin/sh
# build.sh - cross-compile the adapter components for the device.
#
# Usage:
#   ./scripts/build.sh [outdir]        # default outdir: build/
#
# Environment:
#   CROSS         toolchain prefix (default: aarch64-linux-gnu-)
#   GLSTRESS_DIR  glstress repo with SDL headers/libs, for the probes
#                 (default: /home/zmq/projects/glstress)
#
# Outputs:
#   build/libudev.so.1    the fake libudev shim (the core of the adapter)
#   build/jsdump          SDL joystick probe (only if SDL headers/libs found)
#   build/gctest          controller-recognition probe (same condition)
set -e

CROSS="${CROSS:-aarch64-linux-gnu-}"
CC="${CROSS}gcc"
OUT="${1:-build}"
mkdir -p "$OUT"

echo "== building fake libudev shim =="
"$CC" -shared -fPIC -O2 -o "$OUT/libudev.so.1" src/fakeudev.c -ldl

GLSTRESS_DIR="${GLSTRESS_DIR:-/home/zmq/projects/glstress}"
if [ -d "$GLSTRESS_DIR/deps/include/SDL2" ] && [ -d "$GLSTRESS_DIR/lib/SDL2" ]; then
    echo "== building probes (SDL from $GLSTRESS_DIR) =="
    "$CC" -O2 -I "$GLSTRESS_DIR/deps/include/SDL2" -I "$GLSTRESS_DIR/deps/include" \
        -o "$OUT/jsdump" src/jsdump.c \
        -L "$GLSTRESS_DIR/lib/SDL2" -l:libSDL2-2.0.so.0 -Wl,--allow-shlib-undefined
    "$CC" -O2 -I "$GLSTRESS_DIR/deps/include/SDL2" -I "$GLSTRESS_DIR/deps/include" \
        -o "$OUT/gctest" src/gctest.c \
        -L "$GLSTRESS_DIR/lib/SDL2" -l:libSDL2-2.0.so.0 -Wl,--allow-shlib-undefined
else
    echo "== skipping probes: SDL headers/libs not found (set GLSTRESS_DIR) =="
fi

echo "done. artifacts in $OUT/"
ls -la "$OUT"
