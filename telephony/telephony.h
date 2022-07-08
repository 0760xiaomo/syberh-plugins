#ifndef TELEPHONY_H
#define TELEPHONY_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "telephony_global.h"

class TELEPHONYSHARED_EXPORT Telephony: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Telephony();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

};

#endif // TELEPHONY_H
