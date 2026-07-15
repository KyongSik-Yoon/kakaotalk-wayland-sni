#include "actions.hpp"

#include <QDebug>
#include <QProcess>

bool runCommand(const Command &command, const QProcessEnvironment &environment) {
    if (!command.isValid())
        return false;

    QProcess process;
    process.setProcessEnvironment(environment);
    process.start(command.program, command.arguments);
    if (!process.waitForStarted(1000)) {
        qWarning() << "Could not start" << command.program << process.errorString();
        return false;
    }
    if (!process.waitForFinished(3000)) {
        qWarning() << command.program << "did not finish in time";
        process.kill();
        return false;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qWarning() << command.program << "failed with exit code" << process.exitCode();
        return false;
    }
    return true;
}

void focusKakaoTalk(const Config &config) {
    static_cast<void>(runCommand(config.focusCommand, config.actionEnvironment()));
}

void terminateKakaoTalk(const Config &config) {
    static_cast<void>(runCommand(config.quitCommand, config.actionEnvironment()));
}
