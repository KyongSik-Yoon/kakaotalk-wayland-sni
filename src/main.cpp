#include "config.hpp"
#include "status_notifier.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("kakaotalk-wayland-sni"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("StatusNotifierItem companion for KakaoTalk on Wine Wayland"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption printConfig(
        QStringLiteral("print-config"),
        QStringLiteral("Print the effective environment-based configuration and exit."));
    parser.addOption(printConfig);
    parser.process(app);

    Config config = Config::fromEnvironment();
    if (parser.isSet(printConfig)) {
        QTextStream(stdout) << config.summary();
        return 0;
    }

    StatusNotifierService service(std::move(config));
    return app.exec();
}
