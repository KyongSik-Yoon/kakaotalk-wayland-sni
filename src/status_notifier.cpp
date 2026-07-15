#include "status_notifier.hpp"

#include "actions.hpp"
#include "process_detector.hpp"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QTimer>
#include <QVariantMap>

#include <functional>
#include <utility>

namespace {
constexpr auto kServiceName = "org.kde.StatusNotifierItem.kakaotalk";
constexpr auto kItemPath = "/StatusNotifierItem";
constexpr auto kMenuPath = "/MenuBar";
constexpr auto kWatcherService = "org.kde.StatusNotifierWatcher";
constexpr auto kWatcherPath = "/StatusNotifierWatcher";
constexpr auto kWatcherInterface = "org.kde.StatusNotifierWatcher";

struct MenuLayout {
    int id = 0;
    QVariantMap properties;
    QList<QDBusVariant> children;
};

struct MenuProperties {
    int id = 0;
    QVariantMap properties;
};

QDBusArgument &operator<<(QDBusArgument &argument, const MenuLayout &layout) {
    argument.beginStructure();
    argument << layout.id << layout.properties << layout.children;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, MenuLayout &layout) {
    argument.beginStructure();
    argument >> layout.id >> layout.properties >> layout.children;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const MenuProperties &item) {
    argument.beginStructure();
    argument << item.id << item.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, MenuProperties &item) {
    argument.beginStructure();
    argument >> item.id >> item.properties;
    argument.endStructure();
    return argument;
}

QVariantMap menuProperties(int id, bool korean) {
    switch (id) {
    case 1:
        return {{QStringLiteral("label"), korean ? QStringLiteral("열기") : QStringLiteral("Open")},
                {QStringLiteral("icon-name"), QStringLiteral("window")},
                {QStringLiteral("enabled"), true},
                {QStringLiteral("visible"), true}};
    case 2:
        return {{QStringLiteral("type"), QStringLiteral("separator")},
                {QStringLiteral("visible"), true}};
    case 3:
        return {{QStringLiteral("label"), korean ? QStringLiteral("종료") : QStringLiteral("Quit")},
                {QStringLiteral("icon-name"), QStringLiteral("application-exit")},
                {QStringLiteral("enabled"), true},
                {QStringLiteral("visible"), true}};
    default:
        return {{QStringLiteral("children-display"), QStringLiteral("submenu")}};
    }
}

QVariantMap filterProperties(const QVariantMap &properties, const QStringList &names) {
    if (names.isEmpty())
        return properties;
    QVariantMap result;
    for (const QString &name : names) {
        if (properties.contains(name))
            result.insert(name, properties.value(name));
    }
    return result;
}
} // namespace

Q_DECLARE_METATYPE(MenuLayout)
Q_DECLARE_METATYPE(MenuProperties)

class StatusNotifierMenu final : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")

    Q_PROPERTY(uint Version READ version CONSTANT)
    Q_PROPERTY(QString TextDirection READ textDirection CONSTANT)
    Q_PROPERTY(QString Status READ status CONSTANT)
    Q_PROPERTY(QStringList IconThemePath READ iconThemePath CONSTANT)

  public:
    StatusNotifierMenu(bool korean, std::function<void()> open, std::function<void()> quit,
                       QObject *parent = nullptr)
        : QObject(parent), m_korean(korean), m_open(std::move(open)), m_quit(std::move(quit)) {}

    uint version() const {
        return 4;
    }
    QString textDirection() const {
        return QStringLiteral("ltr");
    }
    QString status() const {
        return QStringLiteral("normal");
    }
    QStringList iconThemePath() const {
        return {};
    }

  public slots:
    uint GetLayout(int parentId, int recursionDepth, const QStringList &propertyNames,
                   MenuLayout &layout) {
        layout.id = parentId;
        layout.properties = filterProperties(menuProperties(parentId, m_korean), propertyNames);
        layout.children.clear();
        if (parentId == 0 && recursionDepth != 0) {
            for (const int id : {1, 2, 3}) {
                const MenuLayout child{
                    id, filterProperties(menuProperties(id, m_korean), propertyNames), {}};
                layout.children.append(QDBusVariant(QVariant::fromValue(child)));
            }
        }
        return 1;
    }

    QList<MenuProperties> GetGroupProperties(const QList<int> &ids,
                                             const QStringList &propertyNames) {
        QList<MenuProperties> result;
        const QList<int> selectedIds = ids.isEmpty() ? QList<int>{0, 1, 2, 3} : ids;
        for (const int id : selectedIds)
            result.append({id, filterProperties(menuProperties(id, m_korean), propertyNames)});
        return result;
    }

    QDBusVariant GetProperty(int id, const QString &name) {
        return QDBusVariant(menuProperties(id, m_korean).value(name));
    }

    void Event(int id, const QString &eventId, const QDBusVariant &, uint) {
        if (eventId != QStringLiteral("clicked"))
            return;
        if (id == 1)
            m_open();
        else if (id == 3)
            m_quit();
    }

    bool AboutToShow(int) {
        return false;
    }

  signals:
    void LayoutUpdated(uint revision, int parent);
    void ItemActivationRequested(int id, uint timestamp);

  private:
    bool m_korean;
    std::function<void()> m_open;
    std::function<void()> m_quit;
};

class StatusNotifierItem final : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierItem")

    Q_PROPERTY(QString Category READ category CONSTANT)
    Q_PROPERTY(QString Id READ id CONSTANT)
    Q_PROPERTY(QString Title READ title CONSTANT)
    Q_PROPERTY(QString Status READ status CONSTANT)
    Q_PROPERTY(uint WindowId READ windowId CONSTANT)
    Q_PROPERTY(QString IconName READ iconName CONSTANT)
    Q_PROPERTY(QString OverlayIconName READ overlayIconName CONSTANT)
    Q_PROPERTY(QString AttentionIconName READ attentionIconName CONSTANT)
    Q_PROPERTY(QString IconThemePath READ iconThemePath CONSTANT)
    Q_PROPERTY(bool ItemIsMenu READ itemIsMenu CONSTANT)
    Q_PROPERTY(QDBusObjectPath Menu READ menu CONSTANT)

  public:
    StatusNotifierItem(const Config &config, std::function<void()> activate,
                       QObject *parent = nullptr)
        : QObject(parent), m_config(config), m_activate(std::move(activate)) {}

    QString category() const {
        return QStringLiteral("Communications");
    }
    QString id() const {
        return QStringLiteral("kakaotalk-wayland");
    }
    QString title() const {
        return QStringLiteral("KakaoTalk");
    }
    QString status() const {
        return QStringLiteral("Active");
    }
    uint windowId() const {
        return 0;
    }
    QString iconName() const {
        return m_config.iconName;
    }
    QString overlayIconName() const {
        return {};
    }
    QString attentionIconName() const {
        return {};
    }
    QString iconThemePath() const {
        return {};
    }
    bool itemIsMenu() const {
        return false;
    }
    QDBusObjectPath menu() const {
        return QDBusObjectPath(QString::fromLatin1(kMenuPath));
    }

  public slots:
    void Activate(int, int) {
        m_activate();
    }
    void SecondaryActivate(int x, int y) {
        Activate(x, y);
    }
    void ContextMenu(int, int) {}
    void Scroll(int, const QString &) {}

  signals:
    void NewIcon();
    void NewAttentionIcon();
    void NewOverlayIcon();
    void NewToolTip();
    void NewTitle();
    void NewStatus(const QString &status);
    void NewMenu();

  private:
    const Config &m_config;
    std::function<void()> m_activate;
};

class StatusNotifierService::Private final : public QObject {
    Q_OBJECT

  public:
    explicit Private(Config config, QObject *parent)
        : QObject(parent), config(std::move(config)), bus(QDBusConnection::sessionBus()),
          watcher(QString::fromLatin1(kWatcherService), bus,
                  QDBusServiceWatcher::WatchForRegistration, this),
          menu(
              this->config.menuLanguage == QStringLiteral("ko"),
              [this] { focusKakaoTalk(this->config); },
              [this] { terminateKakaoTalk(this->config); }, this),
          item(this->config, [this] { focusKakaoTalk(this->config); }, this) {
        qDBusRegisterMetaType<MenuLayout>();
        qDBusRegisterMetaType<MenuProperties>();
        qDBusRegisterMetaType<QList<MenuProperties>>();

        connect(&timer, &QTimer::timeout, this, &Private::refresh);
        connect(&watcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString &) {
            if (registered)
                registerWithWatcher();
        });
        timer.start(this->config.pollIntervalMs);
        refresh();
    }

    ~Private() override {
        unregisterItem();
    }

  private slots:
    void refresh() {
        const bool shouldRegister = isNativeWaylandProcessRunning(config.processName);
        if (shouldRegister && !registered)
            registerItem();
        else if (!shouldRegister && registered)
            unregisterItem();
    }

  private:
    void registerItem() {
        if (!bus.isConnected())
            return;
        const auto flags = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals |
                           QDBusConnection::ExportAllProperties;
        if (!bus.registerObject(QString::fromLatin1(kMenuPath), &menu, flags))
            return;
        if (!bus.registerObject(QString::fromLatin1(kItemPath), &item, flags)) {
            bus.unregisterObject(QString::fromLatin1(kMenuPath));
            return;
        }
        if (!bus.registerService(QString::fromLatin1(kServiceName))) {
            bus.unregisterObject(QString::fromLatin1(kItemPath));
            bus.unregisterObject(QString::fromLatin1(kMenuPath));
            return;
        }
        registered = true;
        registerWithWatcher();
    }

    void registerWithWatcher() {
        QDBusInterface statusNotifierWatcher(QString::fromLatin1(kWatcherService),
                                             QString::fromLatin1(kWatcherPath),
                                             QString::fromLatin1(kWatcherInterface), bus);
        if (statusNotifierWatcher.isValid())
            statusNotifierWatcher.asyncCall(QStringLiteral("RegisterStatusNotifierItem"),
                                            QString::fromLatin1(kServiceName));
    }

    void unregisterItem() {
        if (!registered)
            return;
        bus.unregisterService(QString::fromLatin1(kServiceName));
        bus.unregisterObject(QString::fromLatin1(kItemPath));
        bus.unregisterObject(QString::fromLatin1(kMenuPath));
        registered = false;
    }

    Config config;
    QDBusConnection bus;
    QDBusServiceWatcher watcher;
    StatusNotifierMenu menu;
    StatusNotifierItem item;
    QTimer timer;
    bool registered = false;
};

StatusNotifierService::StatusNotifierService(Config config, QObject *parent)
    : QObject(parent), d(new Private(std::move(config), this)) {}

StatusNotifierService::~StatusNotifierService() = default;

#include "status_notifier.moc"
