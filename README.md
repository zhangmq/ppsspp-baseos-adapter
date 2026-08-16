# PPSSPP BaseOS 适配器

[English](README.en.md)

让**官方固件自带的 Anbernic PPSSPP** 在 **BaseOS**（RG35XX-SP / H700 掌机）上
跑通按键的适配栈：手写假 libudev、诊断探针、实际部署配置——完整可复现的档案。

**状态**：已在设备上部署运行。本仓库**仅供存档参考**——未来的正规路线是用
BaseOS 原生 SDL 源码编译 PPSSPP（见下文"原生路线"）。

**范围**：仅在一台 **RG35XX-SP（RGSP、H700、BaseOS）** 上实测通过。
同系列其他设备（RG35XX+/H/Pro、RG40XX、RG28XX、RG34XX 等）**未验证**，
不保证有效。

---

## 为什么需要它

官方 PPSSPP 1.19.3-1053 自带的 **SDL 2.0.12** 依赖 **udev 守护进程**的设备属性库
来枚举摇杆；**BaseOS 没有 udev 守护进程**，SDL 找不到任何摇杆，面键全部失效。
修复方案由四部分组成，随 `PSP.pak/lib/` 部署：

1. SDL 2.0.12 + libasound（提取自官方固件 rootfs）
2. **fake libudev.so.1**（本仓库 `src/fakeudev.c`）：dlopen 拦截垫片，冒充
   udev 守护进程回答 SDL 的查询，只对 `/dev/input/event1` 声明
   ID_INPUT_JOYSTICK/GAMEPAD
3. 真实 libudev.so.1.7.2（垫片其余符号透传）
4. 匹配 SDL 2.0.12 索引体系的 gamecontrollerdb 条目

## 目录结构

```
src/fakeudev.c     假 libudev 垫片源码
src/jsdump.c       摇杆枚举与按键实测工具
src/gctest.c       控制器识别探针（重建版）
scripts/build.sh   交叉编译垫片与探针
scripts/deploy.sh  组装 pak lib 并推送设备
scripts/launch.sh  pak 启动脚本（设备逐字版）
deploy/            设备配置快照（仅官方原样）
```

## 使用方法

### 编译

```sh
./scripts/build.sh              # 垫片 + 探针 -> build/
# CROSS=... GLSTRESS_DIR=...    # 覆盖工具链前缀 / SDL 源码仓库
```

编译 `libudev.so.1`（始终）和探针 `jsdump`/`gctest`（当 `$GLSTRESS_DIR` 下
有 SDL 头文件/库时，默认指向 glstress 仓库）。裸命令（参考用）：

```sh
aarch64-linux-gnu-gcc -shared -fPIC -O2 -o build/libudev.so.1 src/fakeudev.c -ldl
```

`REAL_LIB` 宏按设备上真实 libudev 路径调整。

### 部署

```sh
./scripts/deploy.sh root@<你的设备IP>   # 例如 ./scripts/deploy.sh root@192.168.1.50
# device 必填——填你自己设备的实际 IP（脚本无硬编码 IP）
# 可选 stage 目录（默认 ./vendor）存放三个官方固件产物（不在本仓库）：
#   libSDL2-2.0.so.0 (SDL 2.0.12)、libasound.so.2、libudev.so.1.7.2 (真)
```

脚本会编译垫片、组装四件套 pak `lib/`（假垫片 + 三个官方固件产物）、把
`lib/` 和 `scripts/launch.sh` 推送到设备的 `/mnt/sdcard/Emus/h700/PSP.pak/`，
之后从菜单重新启动 PSP 即可。PPSSPP 本体与配置路径见 `deploy/README.md`。

## 获取官方文件

所有官方产物取自 **Anbernic 官方固件卡**（16G，dmenu 固件，插读卡器即可
只读挂载），分区布局：

- **p5 `linuxrootfs`**（ext4，Ubuntu 22.04 rootfs）：SDL 2.0.12、libasound、libudev
- **p6 `appfs`**（ext4，官方系统里挂载为 `/mnt/vendor`）：官方 PPSSPP 整树

| 文件 | 官方固件位置 | 说明 |
|---|---|---|
| `PPSSPPSDL` + `assets/` + `old/` + `PSP/` | p6 `appfs/deep/ppsspp/` | 官方 PPSSPP 整树；`PSP/BACK/` 是出厂配置基线，`PSP/SYSTEM/` 随运行变化 |
| `libSDL2-2.0.so.0` | p5 `/usr/lib/libSDL2-2.0.so.0.12.0` | 官方 PSP 的 SDL 2.0.12（按 soname 命名） |
| `libasound.so.2` | p5 `/usr/lib/aarch64-linux-gnu/libasound.so.2` | SDL 2.0.12 的 ALSA 依赖 |
| `libudev.so.1.7.2` | p5 `/usr/lib/aarch64-linux-gnu/libudev.so.1.7.2` | 真 libudev（垫片透传目标）；宿主机同版本亦可 |
| `gamecontrollerdb.txt` | 随 `deep/ppsspp` 的 `assets/` | 官方自带 ANBERNIC-keys 条目（含 2.0.12 手柄序索引） |
| memstick（存档） | 设备 `/mnt/mmc/.config/ppsspp` | 运行数据，**非官方固件自带**，从设备 scp |

⚠️ **不要混用**：p5 `/usr/lib/aarch64-linux-gnu/` 下的
`libSDL2-2.0.so.0.2800.5`（2.28.5）是 dmenu 系统的 SDL，不是 PSP 用的
2.0.12——官方 PSP 的 SDL 在 `/usr/lib/`（`libSDL2-2.0.so.0.12.0`）。

获取示例（官方固件卡插读卡器，`sdX` 为实际设备节点）：

```sh
udisksctl mount -b /dev/sdX5 -o ro   # linuxrootfs（SDL / asound / udev）
udisksctl mount -b /dev/sdX6 -o ro   # appfs（PPSSPP 整树）
mkdir -p vendor
cp /run/media/$USER/linuxrootfs/usr/lib/libSDL2-2.0.so.0.12.0 vendor/libSDL2-2.0.so.0
cp /run/media/$USER/linuxrootfs/usr/lib/aarch64-linux-gnu/libasound.so.2 vendor/
cp /run/media/$USER/linuxrootfs/usr/lib/aarch64-linux-gnu/libudev.so.1.7.2 vendor/
cp -r /run/media/$USER/appfs/deep/ppsspp vendor/deep-ppsspp
scp -r root@<设备IP>:/mnt/mmc/.config/ppsspp vendor/memstick-ppsspp/   # memstick 从设备取
```

三个库文件按 soname 命名放入 `vendor/` 后，`./scripts/deploy.sh <device>` 即
可组装部署。

### 探针

```sh
aarch64-linux-gnu-gcc -O2 -I deps/include/SDL2 -I deps/include \
  -o jsdump jsdump.c -L lib/SDL2 -l:libSDL2-2.0.so.0 -Wl,--allow-shlib-undefined
# 在设备上用 pak 会话环境运行（LD_LIBRARY_PATH=... SDL_VIDEODRIVER=mali）
```

`jsdump` 用于实测物理按键对应的 SDL 索引——**不同 SDL 构建的索引体系不同**
（2.28.5 码序 / 2.0.12 手柄序 / 2.0.16 db 是三种体系，gamecontrollerdb 条目
不可混用），每次更换 SDL 构建都要重测。

## 原生路线（优先）

对 **BaseOS 自带 SDL 2.28.5**（`/mnt/sdcard/.system/h700/lib`）编译 PPSSPP——
它原生支持摇杆枚举，不需要假 libudev / SDL 2.0.12 / libasound。使用 2.28.5
索引的 gamecontrollerdb 条目和 controls.ini 规范（设备号 1=键盘、10=手柄、
NKCODE 码）。本仓库此后仅作行为参考。

## NextUI 模拟器注册与启动机制

PSP.pak（以及任何模拟器 pak）如何出现在菜单并启动游戏——设备实测 + NextUI
h700 源码确认：

1. **ROM 目录后缀决定 pak**：`Roms/<Name> (XXX)/` 对应
   `Emus/h700/XXX.pak/launch.sh`。PSP 的目录是 `Roms/PlayStation Portable (PSP)/`
   → `PSP.pak`；游戏 ROM 作为 `$1` 传给 launcher。
2. **启动协议**：菜单把 pak 命令写入 `/tmp/next` 并以退出码 0 退出；会话循环
   （`.system/h700/paks/MinUI.pak/launch.sh`）eval 该命令——应用运行期间前端
   保持关闭，退出后前端重启。会话环境由循环导出（LD_LIBRARY_PATH 前置
   `.system/h700/lib`、`SDL_VIDEODRIVER=mali`、HOME、SDCARD_PATH 等）。
3. **操作警告**：会话循环统计前端非零退出次数，5 次即关机
   （"crash limit reached"）。切勿直接杀 nextui.elf——必须走菜单的
   `/tmp/next` 协议启动应用。
4. **工具类**（无 ROM 的应用，如压测工具）：`Tools/<name>/<Name>.pak/launch.sh`，
   菜单启动不带参数。

## 相关项目

| 项目 | 链接 | 角色 |
|---|---|---|
| **BaseOS** | https://github.com/pvaibhav/BaseOS | 本适配器的目标系统：极简固件（busybox + nextui.elf 前端），即设备当前固件。 |
| **NextUI** | https://github.com/pvaibhav/NextUI (h700 分支) | 通过 pak 机制启动应用的前端 UI。 |
| **PPSSPP** | https://github.com/hrydgard/ppsspp | 上游源码——未来的原生编译将取代本适配器。 |
