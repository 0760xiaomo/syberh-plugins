#include "download.h"
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QDir>
#include <QObject>
#include <QDebug>
#include <QFile>
#include "framework/common/errorinfo.h"
#include "util/validator.h"
#include "helper.h"

using namespace NativeSdk;

//key 既是downloadID也是callbackId
static QMap<QString, TaskInfo*> tasks;
// 防止并发，文件名重复导致数据写入到同一个文件内 先缓存文件名(文件路径) key 文件名(文件路径) val 文件名(文件路径)
static QMap<QString, QString> fileNames;


QString getDownloadPath(DownloadManager::Storage storage) {
    Helper *helper = Helper::instance();
    QString sopid = helper->sopid();

    QString basePath = helper->getInnerStorageRootPath();
    if(storage==DownloadManager::Extended){
        basePath = helper->getExternStorageRootPath();
    }

    QString downloadPath = basePath + "/" + sopid;
    if(!helper->exists(downloadPath)){
        QDir dir(downloadPath);
        dir.mkpath(downloadPath);
    }
    return downloadPath;
}

QJsonObject successJson(const QString &callbackId, const QString &path, int status, qint64 received, qint64 total){
    QJsonObject json;
    json.insert("downloadID", callbackId);
    json.insert("path", path);
    json.insert("status", status);
    json.insert("received", received);
    json.insert("total", total);
    return json;
}


Download::Download()
{
    isnetWork = true;
}

Download::~Download() {
    QMap<QString, TaskInfo*>::ConstIterator it = tasks.begin();
    for(; it!=tasks.end(); it++){
        removeTask(it.key());
    }
    tasks.clear();
    fileNames.clear();
}


void Download::invoke(const QString &callbackId, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "callbackId" << callbackId << "actionName" << actionName << "params" << params << endl;

    if (actionName == "start") {
        start(callbackId, params.value("url").toString(), params.value("name").toString(), params.value("storage").toString());
    } else if (actionName == "cancel"){
        cancel(callbackId, params.value("downloadID").toString());
    } else {
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidCall, ErrorInfo::message(ErrorInfo::InvalidCall, "方法不存在"));
    }
}

void Download::start(const QString &callbackId, const QString &url, const QString &name, const QString &storage){

    qDebug() << Q_FUNC_INFO << "params: " << callbackId << url << name << storage;
    globalCallbackID = callbackId.toLong();

    // 检查网络
//    if (!Validator::netWorkConnected()) {
//        signalManager()->failed(globalCallbackID, ErrorInfo::NetworkError, ErrorInfo::message(ErrorInfo::NetworkError, "请检查网络状态"));
//        globalCallbackID = 0;
//        return;
//    }

    QString downloadName = name;
    if (name.isEmpty()) {
        downloadName = callbackId;
    }

    //检查url
    if(!Validator::isHttpUrl(url)){
        qDebug() << "url parameter is not starts with http or https: " << url << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidParameter, ErrorInfo::message(ErrorInfo::InvalidParameter, "URL无效"));
        globalCallbackID = 0;
        return;
    }

    DownloadManager *downloadManager = new DownloadManager(this);
    //设置下载ID
    downloadManager->setDownloadId(callbackId);
    //设置存储位置
    QString storageLocation = storage.toLower();
    if (storageLocation==STORAGE_INTERNAL) {
        downloadManager->setStorage(DownloadManager::Internal);
    } else if(storageLocation==STORAGE_EXTENDED) {
        downloadManager->setStorage(DownloadManager::Extended);
    }

    connect(downloadManager, &DownloadManager::signalStarted, this, &Download::onStarted);
    connect(downloadManager, &DownloadManager::signalDownloadProcess, this, &Download::onDownloadProcess);
    connect(downloadManager, &DownloadManager::signalReplyFinished, this, &Download::onReplyFinished);
    connect(downloadManager, &DownloadManager::signalDownloadError, this, &Download::onDownloadError);


    TaskInfo *taskInfo = new TaskInfo();
    taskInfo->downloadID = callbackId;
    taskInfo->downloadManager = downloadManager;
    tasks.insert(callbackId, taskInfo);

    //QString basePath = getDownloadPath(downloadManager->getStorage());
    //文件下载到的位置
    QString basePath = storage;
    QString path = basePath + "/" + downloadName;

    // 判断当前文件是否重复，如果重复名称添加序号
    QStringList nameSplit = downloadName.split(".");
    int i = 1;
    while (QFile::exists(path)
           || QFile::exists(path + downloadManager->getDownloadFileSuffix())
           || fileNames.value(path) != NULL) {
        if (nameSplit.size() == 2) {
            // 文件名只有一个小数点的情况
            path = basePath + "/" + nameSplit[nameSplit.size() - 2] + "(" + QString::number(i) + ")." + nameSplit[nameSplit.size() - 1];
        } else if (nameSplit.size() > 2) {
            // 文件名出现多个小数点的情况，nameSplit.size()-2 是数组倒数第2项

            qDebug() << "lastName 倒数第2项的名字: " << nameSplit[nameSplit.size()-2] << endl;

            QString lastName = nameSplit[nameSplit.size()-2].section('(', 0);

            qDebug() << "lastName 倒数第2项的名字--去掉括号后: " << lastName << endl;

            nameSplit.replace(nameSplit.size()-2, lastName + "(" + QString::number(i) + ")");
            path = basePath + "/" + nameSplit.join('.');
        } else {
            // 文件名没有小数点的情况
            path = basePath + "/" + nameSplit[nameSplit.size() - 1] + "(" + QString::number(i) + ")";
        }
        i++;
    }
    // add缓存文件路径
    fileNames.insert(path, path);

    downloadManager->downloadFile(url, path);
}

void Download::cancel(const QString &callbackId, const QString &downloadID) {

    qDebug() << Q_FUNC_INFO << "params: " << callbackId << downloadID;
    globalCallbackID = callbackId.toLong();

    if (downloadID == "") {
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidParameter, ErrorInfo::message(ErrorInfo::InvalidParameter, "downloadID为空"));
        globalCallbackID = 0;
        return;
    }
    if (!tasks.contains(downloadID)) {
        signalManager()->failed(globalCallbackID, ErrorInfo::CannelFailed, ErrorInfo::message(ErrorInfo::CannelFailed, "任务不存在或已完成"));
        globalCallbackID = 0;
        return;
    }
    TaskInfo *taskInfo = tasks.value(downloadID);

    // 删除缓存文件路径
    fileNames.remove(taskInfo->downloadManager->getMPath());

    taskInfo->downloadManager->closeDownload();

    QJsonObject json;
    json.insert("result", true);
    signalManager()->success(globalCallbackID, json);
    globalCallbackID = 0;
}


TaskInfo* Download::findTaskInfo(DownloadManager *downloadManager){
    QMap<QString, TaskInfo*>::ConstIterator it = tasks.begin();
    for(; it!=tasks.end(); it++){
        TaskInfo *taskInfo = it.value();
        if(taskInfo->downloadManager == downloadManager){
            return taskInfo;
        }
    }
    return NULL;
}

void Download::removeTask(const QString &downloadId) {
    qDebug() << Q_FUNC_INFO << "download removeTask " << endl;
    if (tasks.contains(downloadId)) {
        TaskInfo *taskInfo = tasks.value(downloadId);
        if (taskInfo!=NULL) {
            disconnect(taskInfo->downloadManager, &DownloadManager::signalDownloadProcess, this, &Download::onDownloadProcess);
            disconnect(taskInfo->downloadManager, &DownloadManager::signalReplyFinished, this, &Download::onReplyFinished);
            disconnect(taskInfo->downloadManager, &DownloadManager::signalDownloadError, this, &Download::onDownloadError);
            tasks.remove(downloadId);

            taskInfo->downloadManager->deleteLater();
            taskInfo->downloadManager = NULL;

            delete taskInfo;
            taskInfo = NULL;
        }
    }
}


// 更新下载进度;
void Download::onDownloadProcess(const QString &downloadId, const QString &path, qint64 bytesReceived, qint64 bytesTotal) {
    QJsonObject json = successJson(downloadId, path, Downloading, bytesReceived, bytesTotal);
//    qDebug() << Q_FUNC_INFO << "downloadProgress" << json << endl;
    signalManager()->success(downloadId.toLong(), json);
}


void Download::onReplyFinished(const QString &downloadId, const QString &path, int statusCode, const QString &errorMessage){
    qDebug() << Q_FUNC_INFO << "download finished " << statusCode << errorMessage << endl;

    qint64 received = 0;
    qint64 total = 0;
    TaskInfo *taskInfo = tasks.value(downloadId);
    if (taskInfo!=NULL) {
        received = taskInfo->downloadManager->getBytesReceived();
        total = taskInfo->downloadManager->getBytesTotal();
    }

    // 删除缓存文件路径
    fileNames.remove(taskInfo->downloadManager->getMPath());

    // 根据状态码判断当前下载是否出错, 大于等于400算错误
    if (statusCode >= 400) {
        qDebug() << Q_FUNC_INFO << "download failed " << statusCode << errorMessage << endl;
        signalManager()->failed(downloadId.toLong(), ErrorInfo::NetworkError, ErrorInfo::message(ErrorInfo::NetworkError, errorMessage));
    }
    else {
        QJsonObject json = successJson(downloadId, path, Completed, received, total);

        qDebug() << Q_FUNC_INFO << "download success " << statusCode << errorMessage << endl;
        signalManager()->success(downloadId.toLong(), json);
    }
    removeTask(downloadId);
}

void Download::onDownloadError(const QString &downloadId, qint64 code, const QString &error){
    qDebug() << Q_FUNC_INFO << "download failed " << code << error << endl;
    signalManager()->failed(downloadId.toLong(), code, error);
    removeTask(downloadId);
}

void Download::onStarted(const QString &downloadId, const QString &path)
{
    QJsonObject json = successJson(downloadId, path, Started, 0, 0);
    signalManager()->success(downloadId.toLong(), json);
}
