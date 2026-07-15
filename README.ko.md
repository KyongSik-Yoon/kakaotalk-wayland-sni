# kakaotalk-wayland-sni

Wine 네이티브 Wayland 드라이버로 실행한 카카오톡에 Linux
StatusNotifierItem(SNI) 트레이 아이콘을 제공하는 경량 companion입니다.

Wine Wayland 드라이버에서는 카카오톡의 Windows 알림 영역 아이콘이 Linux
트레이로 전달되지 않습니다. 이 프로그램은 네이티브 Wayland로 실행 중인
`KakaoTalk.exe`를 감지하고 D-Bus SNI 아이콘을 대신 등록합니다.

> 이 프로젝트는 Kakao Corp.와 관련 없는 비공식 커뮤니티 프로젝트입니다.
> KakaoTalk 및 관련 상표의 권리는 각 권리자에게 있습니다.

## 기능

- 네이티브 Wayland 카카오톡이 실행 중일 때만 트레이 아이콘 등록
- X11/XWayland 모드에서는 등록하지 않아 `xembedsniproxy` 아이콘과 중복 방지
- 좌클릭 또는 **열기** 메뉴로 카카오톡 창 활성화
- 우클릭 메뉴에서 **열기**, **종료** 제공
- SNI watcher가 재시작되어도 자동 재등록
- systemd 사용자 서비스 자동 실행
- 프로세스명, 창 클래스, Wine prefix, 아이콘, 실행 명령 설정 가능

## 요구 사항

- SNI를 지원하는 Linux 트레이
- 네이티브 Wayland 드라이버를 지원하는 Wine
- Qt 6 Core 및 D-Bus
- 빌드 시 CMake 3.20 이상과 C++20 컴파일러
- 기본 창 활성화 명령을 사용할 경우 Hyprland

Hyprland 이외의 compositor에서는 `KAKAOTALK_SNI_FOCUS_COMMAND`를 설정하면
됩니다.

Arch Linux 빌드 의존성:

```bash
sudo pacman -S --needed base-devel cmake qt6-base
```

## 설치

```bash
git clone <repository-url> kakaotalk-wayland-sni
cd kakaotalk-wayland-sni
./scripts/install.sh
```

설치 스크립트가 빌드와 테스트를 수행하고 `~/.local`에 설치한 뒤
`kakaotalk-wayland-sni.service`를 활성화합니다.

```bash
systemctl --user status kakaotalk-wayland-sni.service
```

## 설정

필요한 경우 아래 파일을 생성합니다.

```text
~/.config/kakaotalk-wayland-sni/config
```

예시:

```bash
KAKAOTALK_SNI_WINE_PREFIX=~/.local/share/kakaotalk
KAKAOTALK_SNI_ICON_NAME=kakaotalk
KAKAOTALK_SNI_MENU_LANGUAGE=ko
KAKAOTALK_SNI_PROCESS_NAME=KakaoTalk.exe
KAKAOTALK_SNI_WINDOW_CLASS=kakaotalk.exe
KAKAOTALK_SNI_POLL_INTERVAL_MS=1000
```

| 환경 변수 | 기본값 | 설명 |
| --- | --- | --- |
| `KAKAOTALK_SNI_PROCESS_NAME` | `KakaoTalk.exe` | `/proc/*/comm`에서 찾을 프로세스명 |
| `KAKAOTALK_SNI_WINDOW_CLASS` | `kakaotalk.exe` | 기본 Hyprland 포커스 명령의 창 클래스 |
| `KAKAOTALK_SNI_WINE_PREFIX` | `~/.local/share/kakaotalk` | 종료할 Wine prefix |
| `KAKAOTALK_SNI_ICON_NAME` | `kakaotalk` | 아이콘 테마에서 찾을 이름 |
| `KAKAOTALK_SNI_MENU_LANGUAGE` | `ko` | 메뉴 표시 언어: `ko` 또는 `en` |
| `KAKAOTALK_SNI_POLL_INTERVAL_MS` | `1000` | 감지 간격, 최소 250ms |
| `KAKAOTALK_SNI_FOCUS_COMMAND` | Hyprland `focuswindow` | 열기 동작에 사용할 전체 명령 |
| `KAKAOTALK_SNI_QUIT_COMMAND` | `wineserver -k` | 종료 동작에 사용할 전체 명령 |

종료 명령에는 `WINEPREFIX` 환경 변수가 전달됩니다. 기본 `wineserver -k`를
사용할 때는 해당 prefix가 카카오톡 전용인지 확인해야 합니다.

실제 적용되는 설정 확인:

```bash
~/.local/bin/kakaotalk-wayland-sni --print-config
```

설정 변경 후:

```bash
systemctl --user restart kakaotalk-wayland-sni.service
```

## 아이콘

이 프로젝트는 카카오톡 이미지 파일을 재배포하지 않습니다. 기본적으로 현재
아이콘 테마의 `kakaotalk` 항목을 사용합니다. 아이콘이 없으면 로컬 아이콘을
설치하거나 `KAKAOTALK_SNI_ICON_NAME`을 기존 아이콘 이름으로 변경하십시오.

## 제거

```bash
./scripts/uninstall.sh
```

사용자 설정 파일은 의도적으로 보존됩니다.

## 라이선스

[MIT](LICENSE)
