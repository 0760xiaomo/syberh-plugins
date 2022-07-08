#include "update.h"
#include "update_p.h"
#include "framework/common/projectconfig.h"
#include "framework/common/errorinfo.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkConfiguration>
#include <QMetaEnum> 
#include <QJsonDocument>
#include <QJsonObject>
#include <csystempackagemanager.h>
#include "cgui_application.h"

// 连接超时(毫秒)
#define U_CONN_TIMEOUT 6000

#define U_SUPPORT_HTTPS(url, request) \
    if (url.startsWith("https", Qt::CaseInsensitive)) { \
        QSslConfiguration config; \
        config.setPeerVerifyMode(QSslSocket::VerifyNone); \
        config.setProtocol(QSsl::UnknownProtocol); \
        request.setSslConfiguration(config); \
    }


// 必须用这个命令空间下的qApp才可以
using namespace SYBEROS;
using namespace NativeSdk;


bool appInfoUrl(QUrl *outUrl)
{
    if(!outUrl){
        return false;
    }
    QString url = ProjectConfig::instance()->getAppStoreUrl();
    if(url.isEmpty()){
        return false;
    }
    QString path = ProjectConfig::instance()->getAppInfoPath();
    if(path.isEmpty()){
        return false;
    }
    QString sopid = ProjectConfig::instance()->getSopid();
    if(sopid.isEmpty()){
        return false;
    }
    outUrl->setUrl(url);
    outUrl->setPath(QString("%1/%2").arg(path, sopid));
    return true;
}

QString openAppStoreUrl()
{
    return "store://mainpage?operation=" + ProjectConfig::instance()->getSopid();
}

void supportSsl(QNetworkRequest &request)
{
    QString scheme = request.url().scheme();
    if(scheme.isEmpty()){
        return;
    }
    if(QString::compare("https", scheme, Qt::CaseInsensitive) == 0){
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::UnknownProtocol);
        request.setSslConfiguration(config);
    }
}

// UpdatePrivate
UpdatePrivate::UpdatePrivate(Update *update) : p(update), m_checking(false)
{
}

UpdatePrivate::~UpdatePrivate()
{
    QObject::disconnect(this);
    p = nullptr;
}

void UpdatePrivate::checkNewVersion(const QString &callbackID)
{
    if(m_checking){
        qDebug() << Q_FUNC_INFO << "checking new version";
        p->signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidCall, "正在检查版本");
        return;
    }
    m_checking = true;
    m_networkAccessManager.reset(new QNetworkAccessManager);
    QNetworkConfiguration configure = m_networkAccessManager->activeConfiguration();
    configure.setConnectTimeout(U_CONN_TIMEOUT);
    m_networkAccessManager->setConfiguration(configure);

    m_callbackID = callbackID.toLong();

    QUrl url;
    if(!appInfoUrl(&url)){
        p->signalManager()->failed(m_callbackID, ErrorInfo::SystemError, "应用商店地址无效");
        return;
    }
    qDebug() << Q_FUNC_INFO << "url:" << url;

    QNetworkRequest request(url);
    supportSsl(request);

    QNetworkReply *reply = m_networkAccessManager->get(request);
    if(reply){
        m_reply.reset(reply);
        connect(reply, &QNetworkReply::finished, this, &UpdatePrivate::onFinished);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    }
}

void UpdatePrivate::openAppStore()
{
    QString url = openAppStoreUrl();
    qDebug() << Q_FUNC_INFO << "url:" << url;
    qApp->openUrl(url);

    p->signalManager()->success(m_callbackID, "");
}

void UpdatePrivate::clear()
{
    m_reply.reset();
    m_networkAccessManager.reset();
    m_checking = false;
}


void UpdatePrivate::onFinished()
{
    QByteArray byteData = m_reply->readAll();

    qDebug() << Q_FUNC_INFO << byteData;

    QJsonParseError err;
    QJsonDocument document = QJsonDocument::fromJson(byteData.constData(), &err);

    if(err.error != QJsonParseError::NoError){
        p->signalManager()->failed(m_callbackID, ErrorInfo::SystemError, "获取响应数据失败: " + err.errorString());
        clear();
        return;
    }
    if(!document.isObject()){
        p->signalManager()->failed(m_callbackID, ErrorInfo::UnknowError, "接口响应数据格式无效");
        clear();
        return;
    }
    QJsonObject object = document.object();

    if(!object.contains("vercode")){
        p->signalManager()->failed(m_callbackID, ErrorInfo::UnknowError, "接口响应数据缺失版本信息");
        clear();
        return;
    }

    CSystemPackageManager pkgManger;
    QSharedPointer<CPackageInfo> info = pkgManger.packageInfo(ProjectConfig::instance()->getSopid());
    if(info.isNull()) {
        p->signalManager()->failed(m_callbackID, ErrorInfo::SystemError, "无法获取当前应用信息");
        clear();
        return;
    }
    int vercode =  object["vercode"].toInt();
    int oldCode = info->versionCode();

    QJsonObject result;
    result.insert("oldCode", oldCode);
    result.insert("vercode", vercode);

    if(vercode > oldCode) {
        result.insert("isNeedUpdate", true);
    } else {
        result.insert("isNeedUpdate", false);
    }
    qDebug() << Q_FUNC_INFO << "result:" << result;
    p->signalManager()->success(m_callbackID, QVariant(result));
    clear();
}

void UpdatePrivate::onError(QNetworkReply::NetworkError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
    QString networkErrMsg = m_reply->errorString();
    QString msg = QString("%1 %2::%3")
        .arg(networkErrMsg.isEmpty()?"":networkErrMsg, metaEnum.name(), metaEnum.valueToKey(static_cast<int>(err)));

    qDebug() << Q_FUNC_INFO << msg;

    p->signalManager()->failed(m_callbackID, ErrorInfo::SystemError, msg);
    clear();
}



// Update
Update::Update()
{
    d.reset(new UpdatePrivate(this));
}

Update::~Update()
{
}

void Update::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "check") {
        d->checkNewVersion(callbackID);
    } else if (actionName == "toStore") {
        d->openAppStore();
    }
}
