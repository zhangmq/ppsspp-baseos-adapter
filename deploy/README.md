# deploy/ — on-device configuration snapshot (official stock only)

Snapshot of the real deployed configuration, pulled from the device
2026-08-16. **Stock files only** — a byte-level diff against the official
firmware (appfs `deep/ppsspp`) confirmed zero modifications; the adapter's
own additions live exclusively in `PSP.pak` (`launch.sh` + `lib/`), never
in the official data. A redundant `gamecontrollerdb.txt.bak` (identical
content) was removed to restore the stock state.

**Note 2026-08-16**: the `AxisSwap = 10-4010` line in
`PSP/SYSTEM/controls.ini` is a **user setting** (enabled manually in the
PPSSPP in-game control options) — neither the adapter nor the stock binary
adds it automatically. After a fresh restore from `PSP/BACK/` it is absent
until the user re-enables it. Treat `PSP/BACK/` as the factory baseline;
`SYSTEM/` drifts with user settings and normal runs (same for `ppsspp.ini`).

设备配置快照（2026-08-16 拉取）。**仅官方原样**——与官方固件（appfs
`deep/ppsspp`）逐字节对照确认零修改；适配层的全部新增都在 `PSP.pak`
（`launch.sh` + `lib/`），从不写入官方数据。冗余的 `gamecontrollerdb.txt.bak`
（内容相同）已移除恢复原样。

**备注（2026-08-16）**：`PSP/SYSTEM/controls.ini` 里的 `AxisSwap = 10-4010`
是**用户手动设置**（PPSSPP 游戏内控制选项里开启的）——适配层和官方二进制
都不会自动添加。从 `PSP/BACK/` 恢复后该行消失，直到用户重新开启。
`PSP/BACK/` 为出厂基线；`SYSTEM/` 随用户设置和正常运行变化（ppsspp.ini 同理）。

| File (this repo) | On-device source | Content |
|---|---|---|
| `launch.sh` | `/mnt/sdcard/Emus/h700/PSP.pak/launch.sh` | pak launcher — verbatim from the device; tracked copy is `../scripts/launch.sh` |
| `controls.ini` | `/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini` | stock `[ControlMapping]` (device 1=keyboard / 10=pad, NKCODE values; the pad lines `10-189/190/188/191` = Cross/Circle/Triangle/Square are stock Anbernic bindings) |
| `ppsspp.ini` | `/mnt/vendor/deep/ppsspp/PSP/SYSTEM/ppsspp.ini` | stock global config (restored from `PSP/BACK/`, the firmware's own factory backup) |
| `gamecontrollerdb.txt` | `/mnt/vendor/deep/ppsspp/assets/gamecontrollerdb.txt` | stock db — the ANBERNIC-keys entry for GUID `19000000010000000100000000010000` (`a:b1,b:b0,x:b2,y:b3,...`, SDL 2.0.12 gamepad-order) is Anbernic's own ("EmuDeep extra gamepads"); `guide:b11` is the stock value, `old/` holds the older `b8` revision |
| `lib/` (in `vendor/`) | `/mnt/sdcard/Emus/h700/PSP.pak/lib/` | libSDL2-2.0.so.0 (2.0.12), libudev.so.1 (built from `src/fakeudev.c`), libudev.so.1.7.2 (real), libasound.so.2 |

The three stock artifacts are **not in this repo** — they live in the
gitignored `vendor/` dir (mirrored from the device / official firmware
rootfs). Name them by soname so `scripts/deploy.sh vendor/` picks them up.
`vendor/deep-ppsspp/` is the full official PPSSPP tree (binary + assets +
PSP/SYSTEM + PSP/BACK + old/) and `vendor/memstick-ppsspp/` the memstick
(saves/cheats/states) — both gitignored, kept for reference and restore.

三个官方固件产物**不在本仓库**，位于 gitignored 的 `vendor/`（设备/官方
固件 rootfs 镜像，按 soname 命名供 `scripts/deploy.sh vendor/` 使用）。
`vendor/deep-ppsspp/` = 官方 PPSSPP 整树（二进制 + assets + PSP/SYSTEM +
PSP/BACK + old/），`vendor/memstick-ppsspp/` = memstick（存档/金手指/状态），
均 gitignored，供参考与恢复。
