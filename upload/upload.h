#ifndef UPLOAD_H
#define UPLOAD_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "upload_global.h"


class UploadResponse;

class UPLOADSHARED_EXPORT Upload: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")
public:
    Upload();
    virtual ~Upload();

    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

public slots:
    void onStarted(const QString &callBackID);
    void onUploadProgress(const QString &callBackID, qint64 bytesSent, qint64 bytesTotal);
    void onFinished(const QString &callBackID, const UploadResponse &response);
    void onError(const QString &callBackID, const QString &error);

private:
    // 上传
    void start(const QString &callbackID, const QVariantMap &params);
    // 取消
    void cancel(const QString &callbackID, const QVariantMap &params);
};

#endif // UPLOAD_H
