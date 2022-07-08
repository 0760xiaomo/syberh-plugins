#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "filepicker_global.h"
#include "qmlmanager.h"


using namespace NativeSdk;

class FILEPICKERSHARED_EXPORT Filepicker: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Filepicker();
    ~Filepicker();

    void invokeInitialize();
    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

    void open(const QString &callbackID, const QVariantMap &params);

private:
    long globalCallbackID;
    QmlManager qmlManager;
    QmlObject *filepickerQml = nullptr;

public slots:
    void chooseOk(const QString &filesPath);
    void chooseCancel();
};

#endif // FILEPICKER_H
