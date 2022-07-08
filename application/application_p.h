#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include <QObject>

#include "iplugin/iplugin.h"

class ApplicationPrivate : public QObject
{
    Q_OBJECT
public:
    ApplicationPrivate(ExtensionSystem::SignalManager *signalManager, QObject *parent = nullptr);
    ~ApplicationPrivate();

    void setAppOrientation(const QString &callbackID, const QVariantMap &params);

    void quit(bool force = false);

private:
    ExtensionSystem::SignalManager *m_signalManager;
};

#endif // APPLICATION_P_H