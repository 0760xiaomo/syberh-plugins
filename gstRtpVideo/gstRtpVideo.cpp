#include "gstRtpVideo.h"
#include "gstRtpVideo_p.h"
#include "framework/common/errorinfo.h"
#include "video/gstrtpplayer.h"
#include "video/videosource.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QVideoFrame>


using namespace NativeSdk;

GstRtpVideo::GstRtpVideo() : d(new GstRtpVideoPrivate(this))
{
    qmlRegisterType<VideoSource>("gstRtpVideo", 1, 0, "VideoSource");
    qmlRegisterType<GstRtpPlayer>("gstRtpVideo", 1, 0, "GstRtpPlayer");
}

GstRtpVideo::~GstRtpVideo()
{
    if(d != nullptr){
        d->deleteLater();
        d = nullptr;
    }
}

void GstRtpVideo::invokeInitialize()
{
}

void GstRtpVideo::invoke(QString callbackID, QString actionName, QVariantMap params)
{
    qDebug() << Q_FUNC_INFO << " callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "create") {
        d->create(callbackID, params);
    } else if(actionName == "remove") {
        d->remove(callbackID, params);
    } else if(actionName == "removeAll"){
        d->removeAll(callbackID);
    } else if(actionName == "start"){
        d->start(callbackID, params);
    } else if(actionName == "stop"){
        d->stop(callbackID, params);
    } else if(actionName == "show"){
        d->show(callbackID, params);
    } else if(actionName == "hide"){
        d->hide(callbackID, params);
    }
}


// ====== GstRtpVideoPrivate ======

GstRtpVideoPrivate::GstRtpVideoPrivate(QObject *parent) : QObject(parent)
{
    p = static_cast<GstRtpVideo*>(parent);
}

GstRtpVideoPrivate::~GstRtpVideoPrivate()
{
    clearQmlObjects();
    p = nullptr;
}

void GstRtpVideoPrivate::create(const QString &callbackID, const QVariantMap &params)
{
    int offsetTop = params.value("offsetTop").toInt();
    int offsetLeft = params.value("offsetLeft").toInt();
    int width = params.value("width").toInt();
    int height = params.value("height").toInt();

    QString address = params.value("address").toString();
    int port = params.value("port").toInt();
    QString format = params.value("format").toString();
    int pixelWidth = params.value("pixelWidth").toInt();
    int pixelHeight = params.value("pixelHeight").toInt();
    bool autoPlay = params.value("autoPlay").toBool();

    qDebug() << "--------------------- >> " << m_qmlObjects;

    QmlObject* qmlObject = m_qmlManager.create("qrc:/qml/SGstRtpVideo.qml", m_qmlManager.currentItem());
    m_qmlObjects.insert(callbackID, qmlObject);

    //绑定信号
    m_qmlManager.connectSignal(qmlObject, SIGNAL(videoEvent(QString, QString, QVariant)), this, SLOT(videoEvent(QString, QString, QVariant)));

    m_qmlManager.connectSignal(qmlObject, SIGNAL(cpagePoped(QString)), this, SLOT(cpagePoped(QString)));


    m_qmlManager.setProperty(qmlObject, "videoId", callbackID);
    m_qmlManager.setProperty(qmlObject, "offsetTop", offsetTop);
    m_qmlManager.setProperty(qmlObject, "offsetLeft", offsetLeft);
    m_qmlManager.setProperty(qmlObject, "width", width);
    m_qmlManager.setProperty(qmlObject, "height", height);
    m_qmlManager.setProperty(qmlObject, "address", address);
    m_qmlManager.setProperty(qmlObject, "port", port);
    m_qmlManager.setProperty(qmlObject, "autoPlay", autoPlay);

    if(format == "NV12"){
        m_qmlManager.setProperty(qmlObject, "format", QVideoFrame::Format_NV12);
    }
    m_qmlManager.setProperty(qmlObject, "pixelWidth", pixelWidth);
    m_qmlManager.setProperty(qmlObject, "pixelHeight", pixelHeight);
    // 开始初始化组件
    m_qmlManager.setProperty(qmlObject, "initVideo", true);

    QJsonObject result;
    result.insert("id", callbackID);
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

void GstRtpVideoPrivate::remove(const QString &callbackID, const QVariantMap &params)
{
    QString id = params.value("id").toString();

    QString err = destoryQmlObject(id);
    if(!err.isEmpty()){
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::PluginError, err);
        return;
    }

    QJsonObject result;
    result.insert("id", id);
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

void GstRtpVideoPrivate::removeAll(const QString &callbackID)
{
    clearQmlObjects();
    p->signalManager()->success(callbackID.toLong(), QVariant());
}

// 开始播放
void GstRtpVideoPrivate::start(const QString &callbackID, const QVariantMap &params)
{
    QString id = params.value("id").toString();

    QmlObject* qmlObject = m_qmlObjects.value(id);
    if(qmlObject == nullptr){
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::PluginError, QString("未找到视频组件 ") + id);
        return;
    }
    QVariant callResult = m_qmlManager.call(qmlObject, "start()");

    QJsonObject result;
    result.insert("id", id);
    result.insert("msg", QJsonValue::fromVariant(callResult));
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

// 停止播放
void GstRtpVideoPrivate::stop(const QString &callbackID, const QVariantMap &params)
{
    QString id = params.value("id").toString();

    QmlObject* qmlObject = m_qmlObjects.value(id);
    if(qmlObject == nullptr){
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::PluginError, QString("未找到视频组件 ") + id);
        return;
    }
    QVariant callResult = m_qmlManager.call(qmlObject, "stop()");

    QJsonObject result;
    result.insert("id", id);
    result.insert("msg", QJsonValue::fromVariant(callResult));
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

// 显示
void GstRtpVideoPrivate::show(const QString &callbackID, const QVariantMap &params)
{
    QString id = params.value("id").toString();

    QmlObject* qmlObject = m_qmlObjects.value(id);
    if(qmlObject == nullptr){
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::PluginError, QString("未找到视频组件 ") + id);
        return;
    }
    m_qmlManager.setProperty(qmlObject, "visible", true);

    QJsonObject result;
    result.insert("id", id);
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

// 隐藏
void GstRtpVideoPrivate::hide(const QString &callbackID, const QVariantMap &params)
{
    QString id = params.value("id").toString();

    QmlObject* qmlObject = m_qmlObjects.value(id);
    if(qmlObject == nullptr){
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::PluginError, QString("未找到视频组件 ") + id);
        return;
    }
    m_qmlManager.setProperty(qmlObject, "visible", false);

    QJsonObject result;
    result.insert("id", id);
    p->signalManager()->success(callbackID.toLong(), QVariant(result));
}

void GstRtpVideoPrivate::clearQmlObjects()
{
    if(m_qmlObjects.isEmpty()){
        return;
    }
    QMapIterator<QString, QmlObject*> i(m_qmlObjects);
    while (i.hasNext()) {
        i.next();
        QmlObject* val = i.value();
        if(val != nullptr){
            m_qmlManager.destroy(val);
        }
    }
    m_qmlObjects.clear();
}

QString GstRtpVideoPrivate::destoryQmlObject(const QString &videoId)
{
    QmlObject* qmlObject = m_qmlObjects.value(videoId);
    if(qmlObject == nullptr){
        return QString("未找到视频组件 ") + videoId;
    }
    m_qmlObjects.remove(videoId);
    m_qmlManager.destroy(qmlObject);
    return QString();
}


void GstRtpVideoPrivate::videoEvent(QString videoId, QString eventType, QVariant eventData)
{
    QVariantMap result;
    result.insert("id", videoId);
    result.insert("type", eventType);
    result.insert("data", eventData);
    result.insert("_toCurrent", true);      // 向当前html发送事件的标志
    p->signalManager()->subscribe("GstRtpVideo", result);
}

void GstRtpVideoPrivate::cpagePoped(QString videoId)
{
    qDebug() << Q_FUNC_INFO << "videoId:" << videoId;
    destoryQmlObject(videoId);
}
