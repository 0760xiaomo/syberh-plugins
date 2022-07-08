#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "application_global.h"


class APPSHARED_EXPORT Application : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Application();
    ~Application();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);
};

#endif // APPLICATION_H
