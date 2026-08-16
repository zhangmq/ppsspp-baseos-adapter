# deploy/ — actual on-device configuration

The real deployed files live on the device (SSH root@192.168.50.233). This
directory gets a snapshot of each once the device is next reachable:

| File (this repo) | On-device source | Content |
|---|---|---|
| `launch.sh` | `/mnt/sdcard/Emus/h700/PSP.pak/launch.sh` | pak launcher: PAK_DIR lib first in LD_LIBRARY_PATH, HOME=/mnt/mmc, `cd /mnt/vendor/deep/ppsspp && exec ./PPSSPPSDL "$ROM" > $LOGS_PATH/PSP.txt` |
| `controls.ini` | `/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini` | `[ControlMapping]` with device ids 1=keyboard / 10=pad, NKCODE values (DPAD 19-22, BUTTON_1..10=188-197); the pad channel lines (`10-189/190/188/191` = Circle/Cross/Triangle/Square) are the working face-button bindings |
| `gamecontrollerdb.txt` | `/mnt/sdcard/.system/h700/ppsspp/assets/gamecontrollerdb.txt` | official entry for GUID `19000000010000000100000000010000`: `a:b1,b:b0,x:b2,y:b3,...` (SDL 2.0.12 gamepad-order indices) |
| `lib/` | `/mnt/sdcard/Emus/h700/PSP.pak/lib/` | libSDL2-2.0.so.0 (2.0.12), libudev.so.1 (built from `src/fakeudev.c`), libudev.so.1.7.2 (real), libasound.so.2 |

Pull with:

```sh
scp root@192.168.50.233:/mnt/sdcard/Emus/h700/PSP.pak/launch.sh deploy/
scp root@192.168.50.233:/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini deploy/
scp root@192.168.50.233:/mnt/sdcard/.system/h700/ppsspp/assets/gamecontrollerdb.txt deploy/
```

The libs are third-party binaries (stock firmware artifacts) — archive them
separately if needed; they are not built from this repo.
