#pragma once

#include <QByteArray>
#include <QString>

[[nodiscard]] bool isNativeWaylandEnvironment(const QByteArray &environment);
[[nodiscard]] bool isNativeWaylandProcessRunning(const QString &processName,
                                                 const QString &procRoot = QStringLiteral("/proc"));
