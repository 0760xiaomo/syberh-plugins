#ifndef UPDATE_H
#define UPDATE_H

#include <QObject>
#include <QtPlugin>
#include <QScopedPointer>

#include "iplugin/iplugin.h"
#include "update_global.h"

class UpdatePrivate;

class UPDATESHARED_EXPORT Update : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Update();
    virtual ~Update();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
    QScopedPointer<UpdatePrivate> d;
};

#endif // UPDATE_H
