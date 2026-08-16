# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@/home/zmq/projects/shared/h700-device.md

@/home/zmq/projects/shared/h700-toolchain.md

@/home/zmq/projects/shared/h700-ppsspp-reference.md

## Overview

Self-contained archive of the **stock-shipped PPSSPP → BaseOS adapter**:
the fake libudev shim source (`src/fakeudev.c`), the diagnostic probes
(`src/jsdump.c`, `src/gctest.c`), and the actual deployed configuration
(`deploy/`). The device runs this stack today via `Emus/h700/PSP.pak`.

**This repo is reference material only.** The sanctioned future path is a
source build of PPSSPP against BaseOS's native SDL 2.28.5 (no hack at all) —
that work lives in `~/projects/ppsspp`. Use this repo to understand the
current deployment, reproduce it, or mine it for behavior references
(button semantics, controls.ini conventions, performance baselines).

Key facts (full detail in the imported shared files):
- The hack exists because SDL 2.0.12 needs a udev daemon's property
  database; BaseOS has none.
- Button semantics target: physical A = Circle = confirm (JP layout).
- SDL index schemes differ per build — 2.28.5 (evdev order) vs 2.0.12
  (gamepad order) vs 2.0.16 db — never mix gamecontrollerdb entries.
- Killing nextui.elf (the menu) triggers the session loop's crash counter
  (5 non-zero exits → device powers off). Run stress/app tests via the
  menu's /tmp/next flow, never by killing the menu.
- Reboot with `/mnt/sdcard/.system/h700/bin/reboot_next`, never busybox
  `reboot` (dirty reboot can wedge the GPU clock at 420MHz).
