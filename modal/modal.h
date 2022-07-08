#ifndef MODAL_H
#define MODAL_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "modal_global.h"
#include "qmlmanager.h"

using namespace NativeSdk;

class MODALSHARED_EXPORT Modal: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Q_INVOKABLE Modal();

    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

    void alert(const QString &callbackID, const QVariantMap &params);
    void confirm(const QString &callbackID, const QVariantMap &params);
    void prompt(const QString &callbackID, const QVariantMap &params);
    void toast(const QString &callbackID, const QVariantMap &params);
    void gtoast(const QString &callbackID, const QVariantMap &params);


private:
    long globalCallbackID;
    QmlManager qmlManager;
    QmlObject *alertQml = nullptr;
    QmlObject *confirmQml = nullptr;
    QmlObject *promptQml = nullptr;
    QmlObject *toastQml = nullptr;


public slots:
    void alertSuccess();
    void confirmSuccess();
    void confirmReject();
    void promptAccepted(const QVariant &value);
    void promptCancel();
};

#endif // MODAL_H
