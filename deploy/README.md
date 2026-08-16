# deploy/ — actual on-device configuration

The real deployed files live on the device (SSH root@192.168.50.233). This
directory gets a snapshot of each once the device is next reachable:

| File (this repo) | On-device source | Content |
|---|---|---|
| `launch.sh` | `/mnt/sdcard/Emus/h700/PSP.pak/launch.sh` | pak launcher — maintained as `../scripts/launch.sh` (reconstructed 2026-08-16); pull the on-device copy verbatim here to re-sync |
| `controls.ini` | `/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini` | `[ControlMapping]` with device ids 1=keyboard / 10=pad, NKCODE values (DPAD 19-22, BUTTON_1..10=188-197); the pad channel lines (`10-189/190/188/191` = Circle/Cross/Triangle/Square) are the working face-button bindings |
| `gamecontrollerdb.txt` | `/mnt/sdcard/.system/h700/ppsspp/assets/gamecontrollerdb.txt` | official entry for GUID `19000000010000000100000000010000`: `a:b1,b:b0,x:b2,y:b3,...` (SDL 2.0.12 gamepad-order indices) |
| `lib/` | `/mnt/sdcard/Emus/h700/PSP.pak/lib/` | libSDL2-2.0.so.0 (2.0.12), libudev.so.1 (built from `src/fakeudev.c`), libudev.so.1.7.2 (real), libasound.so.2 |

The three stock artifacts are **not in this repo** — extract them from the
official firmware (rootfs `/usr/lib/libSDL2-2.0.so.0.12.0` + libasound, and
a host `libudev.so.1.7.2`), or copy the whole dir from the device's
`PSP.pak/lib/`. Name them by soname in a `vendor/` dir so
`scripts/deploy.sh vendor/` picks them up.

三个官方固件产物**不在本仓库**——从官方固件 rootfs 提取（`/usr/lib/`
下的 SDL 2.0.12、libasound 和宿主机的 libudev.so.1.7.2），或直接从设备的
`PSP.pak/lib/` 整目录拷出，按 soname 命名放入 `vendor/` 目录供
`scripts/deploy.sh vendor/` 使用。

Pull with:

```sh
scp root@192.168.50.233:/mnt/sdcard/Emus/h700/PSP.pak/launch.sh deploy/
scp root@192.168.50.233:/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini deploy/
scp root@192.168.50.233:/mnt/sdcard/.system/h700/ppsspp/assets/gamecontrollerdb.txt deploy/
```

The libs are third-party binaries (stock firmware artifacts) — archive them
separately if needed; they are not built from this repo.
