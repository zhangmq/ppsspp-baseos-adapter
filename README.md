# PPSSPP BaseOS Adapter / PPSSPP BaseOS 适配器

The adapter stack that makes the **stock-shipped Anbernic PPSSPP** run with
working buttons on **BaseOS** (RG35XX-SP / H700 handheld): the hand-written
fake libudev shim, diagnostic probes, and the deployed configuration —
as a self-contained, reproducible archive.

让**官方固件自带的 Anbernic PPSSPP** 在 **BaseOS**（RG35XX-SP / H700 掌机）上
跑通按键的适配栈：手写假 libudev、诊断探针、实际部署配置——完整可复现的档案。

**Status / 状态**: deployed and working on the device today. Kept as
**reference material** — the sanctioned future path is a source build of
PPSSPP against BaseOS's native SDL (see "Native way" below).

**状态**：已在设备上部署运行。本仓库**仅供存档参考**——未来的正规路线是用
BaseOS 原生 SDL 源码编译 PPSSPP（见下文"原生路线"）。

---

## Why this exists / 为什么需要它

The official PPSSPP 1.19.3-1053 binary ships with **SDL 2.0.12**, whose Linux
joystick driver relies on the **udev daemon's** device-property database
(ID_INPUT_JOYSTICK). **BaseOS has no udev daemon**, so SDL finds zero
joysticks and all face buttons are dead. The fix ships four parts in
`PSP.pak/lib/`:

官方 PPSSPP 1.19.3-1053 自带的 **SDL 2.0.12** 依赖 **udev 守护进程**的设备属性库
来枚举摇杆；**BaseOS 没有 udev 守护进程**，SDL 找不到任何摇杆，面键全部失效。
修复方案由四部分组成，随 `PSP.pak/lib/` 部署：

1. SDL 2.0.12 + libasound（提取自官方固件 rootfs）
2. **fake libudev.so.1**（本仓库 `src/fakeudev.c`）：dlopen 拦截垫片，冒充
   udev 守护进程回答 SDL 的查询，只对 `/dev/input/event1` 声明
   ID_INPUT_JOYSTICK/GAMEPAD
3. 真实 libudev.so.1.7.2（垫片其余符号透传）
4. 匹配 SDL 2.0.12 索引体系的 gamecontrollerdb 条目

## Layout / 目录结构

```
src/fakeudev.c     The shim (builds to libudev.so.1) / 假 libudev 垫片源码
src/jsdump.c       SDL joystick enumerator + live button reporter / 摇杆枚举与按键实测工具
src/gctest.c       SDL_IsGameController probe (reconstructed) / 控制器识别探针（重建版）
deploy/            Actual on-device config snapshot (pull from device) / 设备真实配置快照
```

## Usage / 使用方法

### Build the shim / 编译垫片

```sh
aarch64-linux-gnu-gcc -shared -fPIC -O2 -o libudev.so.1 src/fakeudev.c -ldl
```

Adjust `REAL_LIB` in the source to the deployed real libudev path.
Adjust `REAL_LIB` 宏为设备上真实 libudev 的路径。

### Deploy / 部署

Ship the four-part stack in the pak's `lib/` dir and prepend it to
LD_LIBRARY_PATH in the pak launcher (see `deploy/README.md` for the pull
commands and the on-device file locations). Runtime libs: SDL 2.0.12 +
fake libudev + real libudev.so.1.7.2 + libasound.so.2; PPSSPP itself stays
in `/mnt/sdcard/.system/h700/ppsspp/` with config at
`/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini`.

四件套放入 pak 的 `lib/`，launcher 里把该目录前置到 LD_LIBRARY_PATH
（拉取命令和设备路径见 `deploy/README.md`）。PPSSPP 本体在
`/mnt/sdcard/.system/h700/ppsspp/`，配置在
`/mnt/vendor/deep/ppsspp/PSP/SYSTEM/controls.ini`。

### Probes / 探针

```sh
aarch64-linux-gnu-gcc -O2 -I deps/include/SDL2 -I deps/include \
  -o jsdump jsdump.c -L lib/SDL2 -l:libSDL2-2.0.so.0 -Wl,--allow-shlib-undefined
# run on device with the pak session env (LD_LIBRARY_PATH=... SDL_VIDEODRIVER=mali)
```

`jsdump` maps physical buttons to SDL indices empirically — use it whenever
the SDL build changes, because **index schemes differ per SDL build**
(2.28.5 evdev order vs 2.0.12 gamepad order vs 2.0.16 db — never mix
gamecontrollerdb entries).

`jsdump` 用于实测物理按键对应的 SDL 索引——**不同 SDL 构建的索引体系不同**
（2.28.5 码序 / 2.0.12 手柄序 / 2.0.16 db 是三种体系，gamecontrollerdb 条目
不可混用），每次更换 SDL 构建都要重测。

## Native way / 原生路线（preferred / 优先）

Compile PPSSPP against **BaseOS's own SDL 2.28.5**
(`/mnt/sdcard/.system/h700/lib`), which natively enumerates the joystick —
no fake libudev, no SDL 2.0.12, no libasound shipping. Use the 2.28.5-index
gamecontrollerdb entry (`a:b3,b:b4,x:b6,y:b5,back:b9,start:b10,guide:b11,...`
+ d-pad hat) and the controls.ini conventions (device 1=keyboard, 10=pad,
NKCODE values). This repo then serves as the behavior reference only.

对 **BaseOS 自带 SDL 2.28.5**（`/mnt/sdcard/.system/h700/lib`）编译 PPSSPP——
它原生支持摇杆枚举，不需要假 libudev / SDL 2.0.12 / libasound。使用 2.28.5
索引的 gamecontrollerdb 条目和 controls.ini 规范（设备号 1=键盘、10=手柄、
NKCODE 码）。本仓库此后仅作行为参考。

## Related projects / 相关项目

| Project / 项目 | URL | Role / 角色 |
|---|---|---|
| **BaseOS** | https://github.com/pvaibhav/BaseOS | The minimal OS this adapter targets — busybox + nextui.elf frontend, the firmware running on the device. 本适配器的目标系统：极简固件（busybox + nextui.elf 前端），即设备当前固件。 |
| **NextUI** | https://github.com/pvaibhav/NextUI (h700 branch) | The frontend UI that launches apps via the pak mechanism (writes /tmp/next, session loop evals it). 通过 pak 机制启动应用的前端 UI。 |
| **PPSSPP** | https://github.com/hrydgard/ppsspp | Upstream source — the future native build replaces this whole adapter. 上游源码——未来的原生编译将取代本适配器。 |
