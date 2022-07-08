#ifndef QRCODE_H
#define QRCODE_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "qrcode_global.h"

class QrcodePrivate;

class QRCODESHARED_EXPORT Qrcode : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")
public:
    explicit Qrcode();
    ~Qrcode();

    void invokeInitialize();
    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

public slots:
    void decodeFinished(const QString &decodeContent);
    void cancel();

private:
    void scan(const QString &callbackID);

    QrcodePrivate *d;
};

#endif // QRCODE_H
