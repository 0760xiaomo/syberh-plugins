#ifndef PATH_H
#define PATH_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "path_global.h"

class PATHSHARED_EXPORT Path: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Path();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :

    void getExternStorageRootPath(const QString &callbackID);
    void getInnerStorageRootPath(const QString &callbackID);
    void getDataRootPath(const QString &callbackID);
};

#endif // PATH_H
