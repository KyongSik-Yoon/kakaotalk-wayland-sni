#pragma once

#include "config.hpp"

#include <QObject>

class StatusNotifierService final : public QObject {
    Q_OBJECT

  public:
    explicit StatusNotifierService(Config config, QObject *parent = nullptr);
    ~StatusNotifierService() override;

  private:
    class Private;
    Private *d;
};
