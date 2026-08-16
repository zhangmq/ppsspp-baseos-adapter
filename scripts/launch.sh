#!/bin/sh
# PSP.pak/launch.sh - launcher for the stock-shipped Anbernic PPSSPP on BaseOS.
#
# Reconstructed 2026-08-16 from session notes (device not reachable for a
# verbatim pull; TODO: refresh from root@192.168.50.233 when online).
# Behavior verified on-device:
#   - pak lib dir FIRST in LD_LIBRARY_PATH: SDL 2.0.12 + fake libudev.so.1 +
#     real libudev.so.1.7.2 + libasound.so.2 live in PSP.pak/lib and MUST
#     shadow the BaseOS system SDL 2.28.5
#   - HOME=/mnt/mmc so PPSSPP's compile-time memstick path
#     (/mnt/mmc/.config/ppsspp) resolves
#   - cd /mnt/vendor/deep/ppsspp, where the official binary + assets live
#     (the internalDataDirectory is compiled into the binary)
#   - exec the binary with the ROM as $1, logging to $LOGS_PATH/PSP.txt
#     ($LOGS_PATH is exported by the session loop: $USERDATA_PATH/logs)

PAK_DIR="$(dirname "$0")"
ROM="$1"
LOG_FILE="$LOGS_PATH/PSP.txt"

[ -d "$LOGS_PATH" ] || mkdir -p "$LOGS_PATH"

export LD_LIBRARY_PATH="$PAK_DIR/lib:$LD_LIBRARY_PATH"
export HOME=/mnt/mmc

cd /mnt/vendor/deep/ppsspp || exit 1

exec ./PPSSPPSDL "$ROM" > "$LOG_FILE" 2>&1
