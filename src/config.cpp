#include "config.hpp"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

namespace {
QString valueOr(const QProcessEnvironment &environment, const QString &name,
                const QString &fallback) {
    const QString value = environment.value(name).trimmed();
    return value.isEmpty() ? fallback : value;
}

QString expandHome(QString path) {
    if (path == QStringLiteral("~"))
        return QDir::homePath();
    if (path.startsWith(QStringLiteral("~/")))
        path.replace(0, 1, QDir::homePath());
    return QDir::cleanPath(path);
}

Command parseCommand(const QString &value, Command fallback) {
    if (value.trimmed().isEmpty())
        return fallback;
    QStringList parts = QProcess::splitCommand(value);
    if (parts.isEmpty())
        return {};
    Command result{parts.takeFirst(), parts};
    return result;
}
} // namespace

Config Config::fromEnvironment(const QProcessEnvironment &environment) {
    Config config;
    config.processName =
        valueOr(environment, QStringLiteral("KAKAOTALK_SNI_PROCESS_NAME"), config.processName);
    config.windowClass =
        valueOr(environment, QStringLiteral("KAKAOTALK_SNI_WINDOW_CLASS"), config.windowClass);
    config.winePrefix =
        expandHome(valueOr(environment, QStringLiteral("KAKAOTALK_SNI_WINE_PREFIX"),
                           QDir::homePath() + QStringLiteral("/.local/share/kakaotalk")));
    config.iconName =
        valueOr(environment, QStringLiteral("KAKAOTALK_SNI_ICON_NAME"), config.iconName);
    config.menuLanguage =
        valueOr(environment, QStringLiteral("KAKAOTALK_SNI_MENU_LANGUAGE"), config.menuLanguage)
            .toLower();
    if (config.menuLanguage != QStringLiteral("ko") && config.menuLanguage != QStringLiteral("en"))
        config.menuLanguage = QStringLiteral("ko");

    bool intervalOk = false;
    const int interval =
        environment.value(QStringLiteral("KAKAOTALK_SNI_POLL_INTERVAL_MS")).toInt(&intervalOk);
    if (intervalOk && interval >= 250)
        config.pollIntervalMs = interval;

    const Command defaultFocus{QStringLiteral("hyprctl"),
                               {QStringLiteral("dispatch"), QStringLiteral("focuswindow"),
                                QStringLiteral("class:^(") +
                                    QRegularExpression::escape(config.windowClass) +
                                    QStringLiteral(")$")}};
    config.focusCommand = parseCommand(
        environment.value(QStringLiteral("KAKAOTALK_SNI_FOCUS_COMMAND")), defaultFocus);
    config.quitCommand =
        parseCommand(environment.value(QStringLiteral("KAKAOTALK_SNI_QUIT_COMMAND")),
                     {QStringLiteral("wineserver"), {QStringLiteral("-k")}});
    return config;
}

QProcessEnvironment Config::actionEnvironment() const {
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("WINEPREFIX"), winePrefix);
    return environment;
}

QString Config::summary() const {
    QString output;
    QTextStream stream(&output);
    stream << "process-name=" << processName << '\n'
           << "window-class=" << windowClass << '\n'
           << "wine-prefix=" << winePrefix << '\n'
           << "icon-name=" << iconName << '\n'
           << "menu-language=" << menuLanguage << '\n'
           << "poll-interval-ms=" << pollIntervalMs << '\n'
           << "focus-command=" << focusCommand.program << ' ' << focusCommand.arguments.join(' ')
           << '\n'
           << "quit-command=" << quitCommand.program << ' ' << quitCommand.arguments.join(' ')
           << '\n';
    return output;
}
