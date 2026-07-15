#pragma once

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

struct Command {
    QString program;
    QStringList arguments;

    [[nodiscard]] bool isValid() const {
        return !program.isEmpty();
    }
};

struct Config {
    QString processName = QStringLiteral("KakaoTalk.exe");
    QString windowClass = QStringLiteral("kakaotalk.exe");
    QString winePrefix;
    QString iconName = QStringLiteral("kakaotalk");
    QString menuLanguage = QStringLiteral("ko");
    int pollIntervalMs = 1000;
    Command focusCommand;
    Command quitCommand;

    static Config fromEnvironment(
        const QProcessEnvironment &environment = QProcessEnvironment::systemEnvironment());
    [[nodiscard]] QProcessEnvironment actionEnvironment() const;
    [[nodiscard]] QString summary() const;
};
