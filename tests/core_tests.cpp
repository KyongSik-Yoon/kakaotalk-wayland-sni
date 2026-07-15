#include "config.hpp"
#include "process_detector.hpp"

#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTextStream>

#include <cstdlib>

namespace {
void require(bool condition, const char *message) {
    if (condition)
        return;
    QTextStream(stderr) << "FAIL: " << message << '\n';
    std::exit(1);
}

void writeFile(const QString &path, const QByteArray &contents) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly), "could not create fake proc file");
    require(file.write(contents) == contents.size(), "could not write fake proc file");
}
} // namespace

int main() {
    QByteArray nativeWayland("WAYLAND_DISPLAY=wayland-1");
    nativeWayland.append('\0');
    nativeWayland.append("XDG_SESSION_TYPE=wayland");
    nativeWayland.append('\0');
    QByteArray xWayland("WAYLAND_DISPLAY=wayland-1");
    xWayland.append('\0');
    xWayland.append("DISPLAY=:0");
    xWayland.append('\0');
    QByteArray x11("DISPLAY=:0");
    x11.append('\0');
    require(isNativeWaylandEnvironment(nativeWayland),
            "native Wayland environment was not detected");
    require(!isNativeWaylandEnvironment(xWayland),
            "XWayland environment must not register a duplicate icon");
    require(!isNativeWaylandEnvironment(x11),
            "X11-only environment must not register a native icon");

    QTemporaryDir fakeProc;
    require(fakeProc.isValid(), "could not create fake proc root");
    require(QDir().mkpath(fakeProc.path() + QStringLiteral("/123")),
            "could not create fake process");
    writeFile(fakeProc.path() + QStringLiteral("/123/comm"), "KakaoTalk.exe\n");
    writeFile(fakeProc.path() + QStringLiteral("/123/environ"), nativeWayland);
    require(isNativeWaylandProcessRunning(QStringLiteral("KakaoTalk.exe"), fakeProc.path()),
            "matching native Wayland KakaoTalk process was not detected");
    require(!isNativeWaylandProcessRunning(QStringLiteral("Other.exe"), fakeProc.path()),
            "unrelated process must not be detected");

    QProcessEnvironment environment;
    environment.insert(QStringLiteral("KAKAOTALK_SNI_PROCESS_NAME"),
                       QStringLiteral("KakaoTalkBeta.exe"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_WINDOW_CLASS"),
                       QStringLiteral("kakaotalk-beta.exe"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_WINE_PREFIX"), QStringLiteral("~/wine/kakao"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_ICON_NAME"), QStringLiteral("custom-kakao"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_MENU_LANGUAGE"), QStringLiteral("en"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_POLL_INTERVAL_MS"), QStringLiteral("2500"));
    environment.insert(QStringLiteral("KAKAOTALK_SNI_FOCUS_COMMAND"),
                       QStringLiteral("custom-focus --name kakao"));
    const Config config = Config::fromEnvironment(environment);
    require(config.processName == QStringLiteral("KakaoTalkBeta.exe"),
            "process name override failed");
    require(config.windowClass == QStringLiteral("kakaotalk-beta.exe"),
            "window class override failed");
    require(config.winePrefix.endsWith(QStringLiteral("/wine/kakao")),
            "Wine prefix home expansion failed");
    require(config.iconName == QStringLiteral("custom-kakao"), "icon override failed");
    require(config.menuLanguage == QStringLiteral("en"), "menu language override failed");
    require(config.pollIntervalMs == 2500, "poll interval override failed");
    require(config.focusCommand.program == QStringLiteral("custom-focus") &&
                config.focusCommand.arguments ==
                    QStringList({QStringLiteral("--name"), QStringLiteral("kakao")}),
            "focus command parsing failed");
    return 0;
}
