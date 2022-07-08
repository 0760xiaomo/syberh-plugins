#include "upload.h"
#include "upload_p.h"
#include "framework/common/errorinfo.h"
#include "util/validator.h"
#include "util/util.h"

#include <QFile>
#include <QJsonObject>
#include <QDebug>


#define UL_STATUS_START         1
#define UL_STATUS_UPLOADING     2
#define UL_STATUS_FINISH        3

#define UL_CHECK_PARAM(params, name, callbackID, msg) \
    if(!NativeSdk::Util::CheckParam(params, name)) { \
        signalManager()->failed(callbackID.toLong(), NativeSdk::ErrorInfo::InvalidParameter, msg); \
        return; \
    }

#define UL_RESULT(var, callbackId, status, sent, total) \
    QJsonObject var; \
    var.insert("id", callbackId); \
    var.insert("status", status); \
    var.insert("sent", sent); \
    var.insert("total", total);


using namespace NativeSdk;

//key 既是downloadID也是callbackID
static QMap<QString, UploadPrivate*> tasks;

void removeTask(const QString &id)
{
    if(id.isEmpty()){
        return;
    }
    UploadPrivate* task = tasks.value(id);
    if(!task){
        return;
    }
    tasks.remove(id);
    QObject::disconnect(task, nullptr, nullptr, nullptr);
    task->deleteLater();
}



Upload::Upload()
{
}

Upload::~Upload()
{
    if(tasks.isEmpty()){
        return;
    }
    QMap<QString, UploadPrivate*>::const_iterator i = tasks.constBegin();
    while (i != tasks.constEnd()) {
        UploadPrivate* private_ = i.value();
        if(private_){
            QObject::disconnect(private_, nullptr, nullptr, nullptr);
            private_->deleteLater();
        }
        ++i;
    }
    tasks.clear();
}


void Upload::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    if (actionName == "start") {
        start(callbackID, params);
    } else if (actionName == "cancel") {
        cancel(callbackID, params);
    }
}

void Upload::start(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params;

    UL_CHECK_PARAM(params, "url", callbackID, "URL为空")
    UL_CHECK_PARAM(params, "file", callbackID, "文件路径为空")

    QString url = params.value("url").toString();
    QString name = params.value("name").toString();
    QString filePath = params.value("file").toString();
    QVariant cookieVar = params.value("cookie");
    QVariant headerVar = params.value("header");
    QVariant formVar = params.value("form");

    if(filePath.startsWith("file://", Qt::CaseInsensitive)){
        filePath = filePath.mid(7);
    }
    if(!QFile::exists(filePath)) {
        qDebug() << Q_FUNC_INFO << "文件地址不存在：" << filePath << endl;
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "文件不存在");
        return;
    }
    // name为空时使用默认值file
    if(name.isEmpty()){
        name = "file";
    }

    UploadPrivate *private_ = new UploadPrivate(callbackID);
    private_->setUrl(url);
    private_->setFile(name, filePath);

    if(cookieVar.canConvert<QVariantMap>()){
        private_->setCookies(cookieVar.toMap());
    }else if(cookieVar.canConvert<QString>()){
        private_->setCookies(cookieVar.toString());
    }
    if(headerVar.isValid() && headerVar.canConvert<QVariantMap>()){
        private_->setHeaders(headerVar.toMap());
    }
    if(formVar.isValid() && formVar.canConvert<QVariantMap>()){
        private_->setForm(formVar.toMap());
    }

    connect(private_, &UploadPrivate::started, this, &Upload::onStarted);
    connect(private_, &UploadPrivate::uploadProcess, this, &Upload::onUploadProgress);
    connect(private_, &UploadPrivate::finished, this, &Upload::onFinished);
    connect(private_, &UploadPrivate::error, this, &Upload::onError);

    private_->upload();
    if(private_->hasError()){
        onError(callbackID, private_->getError());
        disconnect(private_, nullptr, nullptr, nullptr);
        private_->deleteLater();
        return;
    }

    tasks.insert(callbackID, private_);
}


void Upload::cancel(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params;

    UL_CHECK_PARAM(params, "id", callbackID, "id为空")

    QString id = params.value("id").toString();

    UploadPrivate *private_ = tasks.value(id);
    if (private_) {
        private_->abort();
        removeTask(id);

        QJsonObject json;
        json.insert("result", true);
        signalManager()->success(callbackID.toLong(), json);
    } else {
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidCall, "任务不存在或已完成");
    }
}

void Upload::onStarted(const QString &callbackID)
{
    UL_RESULT(result, callbackID, UL_STATUS_START, 0, 0);
    signalManager()->success(callbackID.toLong(), result);
}

void Upload::onUploadProgress(const QString &callbackID, qint64 bytesSent, qint64 bytesTotal)
{
    UL_RESULT(result, callbackID, UL_STATUS_UPLOADING, bytesSent, bytesTotal);
    signalManager()->success(callbackID.toLong(), result);
}

void Upload::onFinished(const QString &callbackID, const UploadResponse &response)
{
    removeTask(callbackID);

    UL_RESULT(result, callbackID, UL_STATUS_FINISH, response.bytesSent, response.bytesTotal);
    result.insert("response", response.toJson());

    signalManager()->success(callbackID.toLong(), result);
}

void Upload::onError(const QString &callbackID, const QString &error)
{
    // 任务异常，删除任务
    removeTask(callbackID);
    signalManager()->failed(callbackID.toLong(), ErrorInfo::SystemError, error);
}

