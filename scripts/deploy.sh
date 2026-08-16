#!/bin/sh
# deploy.sh - assemble the PSP.pak lib stack and push it to the device.
#
# Usage:
#   ./scripts/deploy.sh [stage-dir] [device]
#
#   stage-dir: directory holding the stock-firmware artifacts (NOT in this
#              repo — extract from the official firmware rootfs/appfs, or
#              copy from the device's PSP.pak/lib). Required files:
#                libSDL2-2.0.so.0     SDL 2.0.12 (soname-named)
#                libasound.so.2       ALSA (SDL 2.0.12 dependency)
#                libudev.so.1.7.2     real libudev (shim pass-through)
#              Default: ./vendor
#   device:    ssh target, default root@192.168.50.233
#
# What it does:
#   1. builds the fake libudev shim (scripts/build.sh)
#   2. assembles the pak lib dir (fake shim + the three stock artifacts)
#   3. pushes lib/ and launch.sh to /mnt/sdcard/Emus/h700/PSP.pak/ on the
#      device, then prints the next step (relaunch PSP from the menu).
#
# NOT touched: the PPSSPP binary + assets (/mnt/sdcard/.system/h700/ppsspp),
# controls.ini (/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini), and the
# gamecontrollerdb entry — see deploy/README.md for those.
set -e

STAGE="${1:-vendor}"
DEV="${2:-root@192.168.50.233}"
PAK_REMOTE="/mnt/sdcard/Emus/h700/PSP.pak"

[ -d "$STAGE" ] || { echo "error: stage dir '$STAGE' not found"; exit 1; }

echo "== building shim =="
./scripts/build.sh build

echo "== assembling pak lib =="
rm -rf build/paklib
mkdir -p build/paklib
cp build/libudev.so.1 build/paklib/
for f in libSDL2-2.0.so.0 libasound.so.2 libudev.so.1.7.2; do
    if [ -f "$STAGE/$f" ]; then
        cp "$STAGE/$f" build/paklib/
    else
        echo "error: missing $STAGE/$f (stage the stock-firmware artifacts;"
        echo "       see deploy/README.md for their origin)"
        exit 1
    fi
done
ls -la build/paklib

echo "== pushing to $DEV =="
ssh "$DEV" "mkdir -p $PAK_REMOTE/lib"
scp build/paklib/* "$DEV:$PAK_REMOTE/lib/"
scp scripts/launch.sh "$DEV:$PAK_REMOTE/launch.sh"

echo "== deployed. Relaunch PSP from the menu on the device. =="
