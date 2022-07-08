#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <QtPlugin>
#include <QtNetwork>

#include "iplugin/iplugin.h"
#include "download_global.h"
#include "util/downloadmanager.h"

class TaskInfo{
public:
    // 任务ID
    QString downloadID;
    // 下载
    DownloadManager *downloadManager;
};

#define STORAGE_INTERNAL "internal"
#define STORAGE_EXTENDED "extended"

class DOWNLOADSHARED_EXPORT Download: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")


public:
    //下载状态
    enum ProgressStatus {
        Started = 1,
        Downloading = 2,
        Completed = 3
    };

    Q_INVOKABLE Download();
    ~Download();

    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

private:
    long globalCallbackID;
    
    void start(const QString &callbackId, const QString &url, const QString &name, const QString &storage);

    void cancel(const QString &callbackId, const QString &downloadID);

    TaskInfo* findTaskInfo(DownloadManager *downloadManager);
    //删除TaksInfo对象，断开连接的信号，并移除任务
    void removeTask(const QString &downloadId);

    bool isnetWork;

public slots:

    // 开始下载
    void onStarted(const QString &downloadId, const QString &path);

    void onDownloadProcess(const QString &downloadId, const QString &path, qint64 received, qint64 total);

    void onReplyFinished(const QString &downloadId, const QString &path, int statusCode, const QString &errorMessage);

    void onDownloadError(const QString &downloadId, qint64 statusCode, const QString &error);

};

#endif // DOWNLOAD_H
