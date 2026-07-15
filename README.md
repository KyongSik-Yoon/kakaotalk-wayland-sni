# kakaotalk-wayland-sni

A lightweight StatusNotifierItem companion for KakaoTalk running through Wine's
native Wayland driver.

Wine's Wayland driver does not currently expose KakaoTalk's Windows notification
area icon. This companion detects the native-Wayland `KakaoTalk.exe` process and
publishes an equivalent Linux tray item over D-Bus.

> This is an independent community project. It is not affiliated with or
> endorsed by Kakao Corp. KakaoTalk is a trademark of its respective owner.

[한국어 문서](README.ko.md)

## Features

- Registers an SNI icon only for native Wayland KakaoTalk.
- Avoids a duplicate icon when KakaoTalk is using Wine X11/XWayland.
- Left click or **Open** focuses the KakaoTalk window.
- Right-click menu provides **Open** and **Quit** actions.
- Uses Korean menu labels by default, with an English option.
- Re-registers when the desktop's StatusNotifierWatcher restarts.
- Runs as a lightweight user service without a GUI toolkit process.
- Supports custom process names, icons, Wine prefixes, and action commands.

## Requirements

- Linux with a StatusNotifierItem-compatible tray
- Wine with the native Wayland driver
- Qt 6 Core and D-Bus modules
- CMake 3.20+ and a C++20 compiler to build
- Hyprland for the default focus command

Other Wayland compositors can be used by setting `KAKAOTALK_SNI_FOCUS_COMMAND`.

On Arch Linux, the build dependencies are:

```bash
sudo pacman -S --needed base-devel cmake qt6-base
```

## Install

```bash
git clone https://github.com/KyongSik-Yoon/kakaotalk-wayland-sni.git
cd kakaotalk-wayland-sni
./scripts/install.sh
```

The installer builds and tests the project, installs it under `~/.local`, and
enables `kakaotalk-wayland-sni.service` for the current user.

Check its status with:

```bash
systemctl --user status kakaotalk-wayland-sni.service
```

## Configuration

The service optionally reads:

```text
~/.config/kakaotalk-wayland-sni/config
```

Example:

```bash
KAKAOTALK_SNI_WINE_PREFIX=~/.local/share/kakaotalk
KAKAOTALK_SNI_ICON_NAME=kakaotalk
KAKAOTALK_SNI_MENU_LANGUAGE=ko
KAKAOTALK_SNI_PROCESS_NAME=KakaoTalk.exe
KAKAOTALK_SNI_WINDOW_CLASS=kakaotalk.exe
KAKAOTALK_SNI_POLL_INTERVAL_MS=1000
```

Available variables:

| Variable | Default | Purpose |
| --- | --- | --- |
| `KAKAOTALK_SNI_PROCESS_NAME` | `KakaoTalk.exe` | Process name read from `/proc/*/comm` |
| `KAKAOTALK_SNI_WINDOW_CLASS` | `kakaotalk.exe` | Window class used by the default Hyprland command |
| `KAKAOTALK_SNI_WINE_PREFIX` | `~/.local/share/kakaotalk` | Prefix targeted by the Quit action |
| `KAKAOTALK_SNI_ICON_NAME` | `kakaotalk` | Freedesktop icon-theme name |
| `KAKAOTALK_SNI_MENU_LANGUAGE` | `ko` | Menu labels: `ko` or `en` |
| `KAKAOTALK_SNI_POLL_INTERVAL_MS` | `1000` | Detection interval; minimum 250 ms |
| `KAKAOTALK_SNI_FOCUS_COMMAND` | Hyprland `focuswindow` command | Full command used by Open |
| `KAKAOTALK_SNI_QUIT_COMMAND` | `wineserver -k` | Full command used by Quit |

The Quit command receives `WINEPREFIX` in its environment. Be sure the prefix is
dedicated to KakaoTalk before using the default `wineserver -k` action.

Print the effective configuration:

```bash
~/.local/bin/kakaotalk-wayland-sni --print-config
```

After changing the configuration, restart the service:

```bash
systemctl --user restart kakaotalk-wayland-sni.service
```

## Icon

The project does not redistribute KakaoTalk artwork. It asks the current icon
theme for the configured name (`kakaotalk` by default). If the tray shows a
generic or missing icon, install an icon locally or set
`KAKAOTALK_SNI_ICON_NAME` to an existing icon-theme entry.

## Manual build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Uninstall

```bash
./scripts/uninstall.sh
```

The uninstall script intentionally keeps the configuration file.

## How detection works

Once per second, the companion scans same-user process metadata in `/proc`. It
registers the SNI only when a matching process has a non-empty
`WAYLAND_DISPLAY` and no non-empty `DISPLAY`. This separates native Wine Wayland
from the X11/XWayland mode where `xembedsniproxy` can already expose the icon.

## License

[MIT](LICENSE)
