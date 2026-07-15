#include "process_detector.hpp"

#include <QDir>
#include <QFile>

namespace {
bool isNumeric(const QString &value) {
    if (value.isEmpty())
        return false;
    for (const QChar character : value) {
        if (!character.isDigit())
            return false;
    }
    return true;
}
} // namespace

bool isNativeWaylandEnvironment(const QByteArray &environment) {
    bool hasWaylandDisplay = false;
    bool hasX11Display = false;
    for (const QByteArray &variable : environment.split('\0')) {
        if (variable.startsWith("WAYLAND_DISPLAY=") && variable.size() > 16)
            hasWaylandDisplay = true;
        else if (variable.startsWith("DISPLAY=") && variable.size() > 8)
            hasX11Display = true;
    }
    return hasWaylandDisplay && !hasX11Display;
}

bool isNativeWaylandProcessRunning(const QString &processName, const QString &procRoot) {
    const QDir proc(procRoot);
    const QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        if (!isNumeric(entry))
            continue;

        QFile comm(proc.filePath(entry + QStringLiteral("/comm")));
        if (!comm.open(QIODevice::ReadOnly) ||
            QString::fromUtf8(comm.readAll()).trimmed() != processName)
            continue;

        QFile environ(proc.filePath(entry + QStringLiteral("/environ")));
        if (environ.open(QIODevice::ReadOnly) && isNativeWaylandEnvironment(environ.readAll()))
            return true;
    }
    return false;
}
