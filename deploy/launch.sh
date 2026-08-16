#!/bin/sh
# PSP.pak - official PPSSPP 1.19.3-1053, full stock stack replicated:
# binary/assets/db at /mnt/vendor/deep/ppsspp (verbatim snapshot), memstick at
# /mnt/mmc/.config/ppsspp, and lib/ carrying the stock SDL 2.0.12 + libudev +
# libasound (SDL 2.0.12 needs libudev to enumerate the joystick; BaseOS lacks
# it system-wide). Button semantics = stock firmware: A=Circle(confirm),
# B=Cross, X=Square, Y=Triangle. HOME=/mnt/mmc keeps the memstick layout.

EMU_TAG=$(basename "$(dirname "$0")" .pak)
ROM="$1"
PPSSPP_DIR="/mnt/vendor/deep/ppsspp"
PAK_DIR="/mnt/sdcard/Emus/h700/PSP.pak"
mkdir -p "$SAVES_PATH/$EMU_TAG" "$USERDATA_PATH/$EMU_TAG"
export HOME="/mnt/mmc"
export LD_LIBRARY_PATH="$PAK_DIR/lib:/mnt/sdcard/.system/h700/lib:/usr/lib:$LD_LIBRARY_PATH"
cd "$PPSSPP_DIR" || exit 1
exec ./PPSSPPSDL "$ROM" > "$LOGS_PATH/$EMU_TAG.txt" 2>&1
