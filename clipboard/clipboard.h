#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "clipboard_global.h"

class CLIPBOARDSHARED_EXPORT Clipboard: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Q_INVOKABLE Clipboard();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);
    
    void setClipboardData(const QString &callbackID, const QVariantMap &params);
    void getClipboardData(const QString &callbackID, const QVariantMap &params);
  
};

#endif // CLIPBOARD_H
