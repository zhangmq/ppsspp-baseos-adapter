# PPSSPP BaseOS Adapter

[中文](README.md)

The adapter stack that makes the **stock-shipped Anbernic PPSSPP** run with
working buttons on **BaseOS** (RG35XX-SP / H700 handheld): the hand-written
fake libudev shim, diagnostic probes, and the deployed configuration —
as a self-contained, reproducible archive.

**Status**: deployed and working on the device today. Kept as
**reference material** — the sanctioned future path is a source build of
PPSSPP against BaseOS's native SDL (see "Native way" below).

**Scope**: tested on **one RG35XX-SP (RGSP, H700, BaseOS) only**.
Same-family devices (RG35XX Plus/H/Pro, RG40XX, RG28XX, RG34XX, ...) are
**unverified** — expect that this may or may not work on them.

---

## Why this exists

The official PPSSPP 1.19.3-1053 binary ships with **SDL 2.0.12**, whose Linux
joystick driver relies on the **udev daemon's** device-property database
(ID_INPUT_JOYSTICK). **BaseOS has no udev daemon**, so SDL finds zero
joysticks and all face buttons are dead. The fix ships four parts in
`PSP.pak/lib/`:

1. SDL 2.0.12 + libasound (extracted from the official firmware rootfs)
2. **fake libudev.so.1** (this repo's `src/fakeudev.c`): a dlopen-interposing
   shim that answers SDL's udev queries, declaring
   ID_INPUT_JOYSTICK/GAMEPAD for `/dev/input/event1` only
3. real libudev.so.1.7.2 (pass-through for the shim's other symbols)
4. a gamecontrollerdb entry matching the SDL 2.0.12 index scheme

## Layout

```
src/fakeudev.c     The shim (builds to libudev.so.1)
src/jsdump.c       SDL joystick enumerator + live button reporter
src/gctest.c       SDL_IsGameController probe (reconstructed)
scripts/build.sh   Cross-compile shim + probes
scripts/deploy.sh  Assemble the pak lib stack and push to the device
scripts/launch.sh  PSP.pak launcher (verbatim from the device, 2026-08-16)
deploy/            On-device config snapshot — official stock files only
```

## Usage

### Build

```sh
./scripts/build.sh              # shim + probes -> build/
# CROSS=... GLSTRESS_DIR=...    # override toolchain prefix / SDL source repo
```

This cross-compiles `libudev.so.1` from `src/fakeudev.c` (always) and the
probes `jsdump`/`gctest` (when SDL headers/libs are found in `$GLSTRESS_DIR`,
defaulting to the glstress repo). Raw command for reference:

```sh
aarch64-linux-gnu-gcc -shared -fPIC -O2 -o build/libudev.so.1 src/fakeudev.c -ldl
```

Adjust `REAL_LIB` in the source to the deployed real libudev path.

### Deploy

```sh
./scripts/deploy.sh root@<your-device-ip>   # e.g. ./scripts/deploy.sh root@192.168.1.50
# device is required — set YOUR device's actual IP (no hardcoded IP)
# optional stage dir (default ./vendor) holds the three stock-firmware
# artifacts (not in this repo):
#   libSDL2-2.0.so.0 (SDL 2.0.12), libasound.so.2, libudev.so.1.7.2 (real)
```

The script builds the shim, assembles the four-part pak `lib/` (fake shim +
the three stock artifacts), and pushes `lib/` + `scripts/launch.sh` to
`/mnt/sdcard/Emus/h700/PSP.pak/` on the device; relaunch PSP from the menu
afterwards. PPSSPP itself stays in `/mnt/vendor/deep/ppsspp/` (official
binary + assets) with config at `/mnt/vendor/deep/ppsspp/PSP/SYSTEM/` — see
`deploy/README.md` for those file locations and the artifact origins.

### Probes

```sh
aarch64-linux-gnu-gcc -O2 -I deps/include/SDL2 -I deps/include \
  -o jsdump jsdump.c -L lib/SDL2 -l:libSDL2-2.0.so.0 -Wl,--allow-shlib-undefined
# run on device with the pak session env (LD_LIBRARY_PATH=... SDL_VIDEODRIVER=mali)
```

`jsdump` maps physical buttons to SDL indices empirically — use it whenever
the SDL build changes, because **index schemes differ per SDL build**
(2.28.5 evdev order vs 2.0.12 gamepad order vs 2.0.16 db — never mix
gamecontrollerdb entries).

## Native way (preferred)

Compile PPSSPP against **BaseOS's own SDL 2.28.5**
(`/mnt/sdcard/.system/h700/lib`), which natively enumerates the joystick —
no fake libudev, no SDL 2.0.12, no libasound shipping. Use the 2.28.5-index
gamecontrollerdb entry (`a:b3,b:b4,x:b6,y:b5,back:b9,start:b10,guide:b11,...`
+ d-pad hat) and the controls.ini conventions (device 1=keyboard, 10=pad,
NKCODE values). This repo then serves as the behavior reference only.

## How NextUI registers & launches emulators

How PSP.pak (and any emulator pak) gets into the menu and runs a game —
verified on the device / from the NextUI h700 source:

1. **Roms folder naming decides the pak**: a folder `Roms/<Name> (XXX)/` maps
   to `Emus/h700/XXX.pak/launch.sh`. For PSP the folder is
   `Roms/PlayStation Portable (PSP)/` → `PSP.pak`; the game ROM is passed to
   the launcher as `$1`.
2. **Launch protocol**: the menu writes the pak command to `/tmp/next` and
   exits with code 0; the session loop (`MinUI.pak/launch.sh` in
   `.system/h700/paks/`) evals it — the frontend stays down while the app
   runs, then restarts. The session loop exports the environment
   (LD_LIBRARY_PATH with `.system/h700/lib` first, `SDL_VIDEODRIVER=mali`,
   HOME, SDCARD_PATH, ...).
3. **Operational warning**: the session loop counts non-zero frontend exits
   and powers the device off after 5 ("crash limit reached"). Never kill
   nextui.elf directly — always launch apps through the menu's `/tmp/next`
   flow.
4. **Tools** (apps without ROMs, e.g. stress tools):
   `Tools/<name>/<Name>.pak/launch.sh`, launched from the menu without
   arguments.

## Related projects

| Project | URL | Role |
|---|---|---|
| **BaseOS** | https://github.com/pvaibhav/BaseOS | The minimal OS this adapter targets — busybox + nextui.elf frontend, the firmware running on the device. |
| **NextUI** | https://github.com/pvaibhav/NextUI (h700 branch) | The frontend UI that launches apps via the pak mechanism (writes /tmp/next, session loop evals it). |
| **PPSSPP** | https://github.com/hrydgard/ppsspp | Upstream source — the future native build replaces this whole adapter. |
