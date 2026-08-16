# ppsspp-baseos-adapter

The adapter stack that makes the **stock-shipped Anbernic PPSSPP** run with
working buttons on **BaseOS** (RG35XX-SP / H700), as a self-contained archive:
the hand-written fake libudev shim, the diagnostic probes, and the actual
deployed configuration.

**Status: reference/archive.** The current hack works and is deployed, but a
source build of PPSSPP against BaseOS's native SDL 2.28.5 is the sanctioned
future path (see the "native way" notes below and the shared project
knowledge) — this repo then serves as the behavior reference.

## Why this exists

The official PPSSPP 1.19.3-1053 binary ships with **SDL 2.0.12**, whose Linux
joystick driver depends on the **udev daemon's** device-property database.
BaseOS has no udev daemon, so SDL finds zero joysticks and all face buttons
are dead. The fix is a four-part stack shipped in `PSP.pak/lib/`:

1. SDL 2.0.12 + libasound (extracted from the stock firmware rootfs)
2. **fake libudev.so.1** (this repo's `src/fakeudev.c`): dlopen-intercept
   shim that answers the udev queries SDL 2.0.12 makes, claiming
   ID_INPUT_JOYSTICK/GAMEPAD for `/dev/input/event1` only
3. Real libudev.so.1.7.2 (the shim passes everything else through)
4. A gamecontrollerdb entry matching SDL 2.0.12's index scheme

## Layout

- `src/fakeudev.c` — the shim (builds to `libudev.so.1`, ~26 udev symbols
  with leading-underscore aliases for SDL 2.0.12's dlsym style)
- `src/jsdump.c` — SDL joystick enumerator + live button-press reporter
  (used to map physical buttons to SDL indices empirically)
- `src/gctest.c` — SDL_IsGameController probe (reconstructed from notes;
  original one-off was not archived)
- `deploy/` — the actual on-device configuration, pulled from the device:
  the pak launcher, the controls.ini mapping file, the gamecontrollerdb entry
- `docs/` — how it works and how to reproduce

## Build the shim (cross-compile on host)

```sh
aarch64-linux-gnu-gcc -shared -fPIC -O2 -o libudev.so.1 src/fakeudev.c -ldl
```

(Adjust `REAL_LIB` in the source to the deployed real libudev path.)

## Native way (preferred going forward)

Compile PPSSPP against BaseOS's own SDL 2.28.5 (`/mnt/sdcard/.system/h700/lib`),
which natively enumerates the joystick — no fake libudev, no SDL 2.0.12, no
libasound shipping. Use the 2.28.5-index gamecontrollerdb entry
(`a:b3,b:b4,x:b6,y:b5,back:b9,start:b10,guide:b11,...` + d-pad hat) and the
controls.ini conventions documented in the shared project knowledge. See
`~/projects/shared/h700-ppsspp-reference.md` for the full reference.
