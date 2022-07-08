#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtPlugin>

#include "network_p.h"
#include "iplugin/iplugin.h"
#include "network_global.h"


class NetworkTasks;

class NETWORKSHARED_EXPORT Network: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Network();
    ~Network();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
    QScopedPointer<NetworkTasks> m_networkTasks;

public slots:
    void finished(const QString &callbackID, const NetworkResponse &response);
    void error(const QString &callbackID, const QString &err);
};

#endif // NETWORK_H
