#include "network_p.h"
#include "util/validator.h"
#include "util/util.h"
#include "networkcookiejar.h"

#include <QMetaEnum>
#include <QBuffer>
#include <QDebug>
#include <QJsonDocument>


using namespace NativeSdk;

#define NET_P_SUPPORT_HTTPS(url, request) \
    if (url.startsWith("https", Qt::CaseInsensitive)) { \
        QSslConfiguration config; \
        config.setPeerVerifyMode(QSslSocket::VerifyNone); \
        config.setProtocol(QSsl::UnknownProtocol); \
        request.setSslConfiguration(config); \
    }

#define NET_P_REPLY_DISCONNECT(reply) \
    if(reply){ \
        QObject::disconnect(reply, nullptr, nullptr, nullptr); \
    }

#define NET_P_REPLY_CONNECT(reply) \
    connect(reply, &QNetworkReply::finished, this, &NetworkPrivate::onFinished); \
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));


static const int maxRedirectTotal = 5;
static int redirectCount = 0;


NetworkPrivate::NetworkPrivate(const QString &callbackID, QObject *parent) : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_cookieJar(new NetworkCookieJar(this))
    , m_callbackID(callbackID)
{
}


NetworkPrivate::~NetworkPrivate()
{
    NET_P_REPLY_DISCONNECT(m_reply.data())
}


NetworkPrivate* NetworkPrivate::New(const QString &callbackID, QObject *parent)
{
    return new NetworkPrivate(callbackID, parent);
}


NetworkPrivate* NetworkPrivate::setCookies(const QVariantMap &cookies)
{
    if(cookies.isEmpty()){
        return this;
    }
    QMapIterator<QString,QVariant> it(cookies);
    while(it.hasNext()){
        auto item = it.next();
        m_cookies.push_back(QNetworkCookie(item.key().toUtf8(), item.value().toByteArray()));
    }
    return this;
}


NetworkPrivate* NetworkPrivate::setCookies(const QString &cookies)
{
    if(cookies.isEmpty()){
        return this;
    }
    m_cookies = QNetworkCookie::parseCookies(cookies.toUtf8());
    return this;
}


NetworkPrivate* NetworkPrivate::setData(const QMap<QString,QString> &data)
{
    QMap<QString,QString>::ConstIterator it = data.constBegin();
    QString content;
    while (it != data.constEnd()) {
        content += QString("%1=%2").arg(it.key(), it.value());
        it++;
        if (it != data.constEnd()) {
            content.append("&");
        }
    }
    m_data = content;
    return this;
}

NetworkPrivate* NetworkPrivate::setData(const QMap<QString, QVariant> &data)
{
    QMap<QString, QVariant>::ConstIterator it = data.constBegin();
    QString content;
    while (it != data.constEnd()) {
        content += QString("%1=%2").arg(it.key(), it.value().toString());
        it++;
        if (it != data.constEnd()) {
            content.append("&");
        }
    }
    m_data = content;
    return this;
}


void NetworkPrivate::request()
{
    qDebug() << Q_FUNC_INFO;
    if(hasError()){
        return;
    }
    QNetworkRequest req;
    if(!setRequestUrl(req)){
        return;
    }
    QString requestMethod;
    if(!checkRequestMethod(&requestMethod)){
        return;
    }
    if(!setRequestHeaders(req, m_headers)){
        return;
    }
    if(!setRequestCookies(req)){
        return;
    }
    QByteArray requestData;
    if(!checkRequestData(&requestData)){
        return;
    }

    QNetworkReply *reply = m_manager->sendCustomRequest(req, requestMethod.toLatin1(), requestData);
    if(reply){
        NET_P_REPLY_CONNECT(reply)
        m_reply.reset(reply);
    }
}


void NetworkPrivate::redirect(const QString &url, const QByteArray &cookieString)
{
    qDebug() << Q_FUNC_INFO << "url:" << url;

    QNetworkRequest request(url);
    NET_P_SUPPORT_HTTPS(url, request)

    if(!cookieString.isEmpty()){
        QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(cookieString);
        m_cookieJar->updateCookies(cookies);
    }
    m_manager->setCookieJar(m_cookieJar.data());
    QNetworkReply *reply = m_manager->get(request);
    if(reply){
        NET_P_REPLY_DISCONNECT(m_reply.data())
        NET_P_REPLY_CONNECT(reply)
        m_reply.reset(reply);
    }
}


bool NetworkPrivate::setRequestUrl(QNetworkRequest &request)
{
    qDebug() << Q_FUNC_INFO;
    //检查url
    if(m_url.isEmpty()){
        setError("URL为空");
        return false;
    }
    if (!Validator::isHttpUrl(m_url)) {
        setError("URL无效");
        return false;
    }

    NET_P_SUPPORT_HTTPS(m_url, request)

    request.setUrl(m_url);
    return true;
}


bool NetworkPrivate::checkRequestMethod(QString *outMethod)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<NetworkPrivate::Method>();
    for(int i=0; i<metaEnum.keyCount(); i++){
        if(metaEnum.value(i) == static_cast<int>(m_method)){
            *outMethod = metaEnum.key(i);
            return true;
        }
    }
    setError("method无效");
    return false;
}


bool NetworkPrivate::setRequestHeaders(QNetworkRequest &request, const QVariantMap &headers)
{
    if(headers.isEmpty()){
        return true;
    }
    bool hasContentType = false;
    QVariantMap::ConstIterator it = headers.constBegin();
    while (it != headers.constEnd()) {
        QString headerName = it.key();
        request.setRawHeader(headerName.toUtf8(), it.value().toString().toUtf8());
        hasContentType = (Util::Equals(headerName, "Content-Type", true) || Util::Equals(headerName, "ContentType", true));
        it++;
    }
    if (!hasContentType) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }
    return true;
}


bool NetworkPrivate::checkRequestData(QByteArray *outData)
{
    if(m_data.isEmpty()){
        return true;
    }
    *outData = m_data.toUtf8();
    return true;
}

bool NetworkPrivate::setRequestCookies(QNetworkRequest &request)
{
    if(m_cookies.isEmpty()){
        return true;
    }
    m_cookieJar->setCookies(m_cookies);
    m_manager->setCookieJar(m_cookieJar.data());

    QVariant var;
    var.setValue(m_cookies);
    request.setHeader(QNetworkRequest::CookieHeader, var);
    return true;
}

NetworkPrivate::Method NetworkPrivate::toMethod(const QString &method)
{
    if(method.isEmpty()){
        return defaultMethod;
    }
    QMetaEnum metaEnum = QMetaEnum::fromType<NetworkPrivate::Method>();
    for(int i=0; i<metaEnum.keyCount(); i++){
        if(Util::Equals(method, metaEnum.key(i), true)){
            return static_cast<NetworkPrivate::Method>(metaEnum.value(i));
        }
    }
    setError("method无效");
    return defaultMethod;
}


NetworkPrivate::DataType NetworkPrivate::toDataType(const QString &dataType)
{
    if(dataType.isEmpty()){
        return TEXT;
    }
    QMetaEnum metaEnum = QMetaEnum::fromType<NetworkPrivate::DataType>();
    for(int i=0; i<metaEnum.keyCount(); i++){
        if(Util::Equals(dataType, metaEnum.key(i), true)){
            return static_cast<NetworkPrivate::DataType>(metaEnum.value(i));
        }
    }
    setError("dataType无效");
    return TEXT;
}


void NetworkPrivate::onFinished()
{
    if(m_reply.isNull()){
        return;
    }
    QVariant statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    qDebug() << Q_FUNC_INFO << " statusCode: " << statusCode << " error: " << m_reply->error();
    if(m_reply->error() != QNetworkReply::NoError){
        onError(m_reply->error());
        return;
    }
    // 重定向
    if(statusCode.toInt() == 302 && m_autoRedirect){
        if(redirectCount >= maxRedirectTotal){
            QString msg = QString("超过最大重定向次数：{1}/{2}").arg(redirectCount).arg(maxRedirectTotal);
            setError(msg);
            emit error(m_callbackID, msg);
            return;
        }
        QVariant locationVar = m_reply->header(QNetworkRequest::LocationHeader);
        if(locationVar.isNull() || !locationVar.isValid()){
            QString msg("未能获取响应头Location，无法完成重定向");
            setError(msg);
            emit error(m_callbackID, msg);
            return;
        }
        redirectCount++;

        QVariant setCookieVar = m_reply->header(QNetworkRequest::SetCookieHeader);
        if(setCookieVar.isNull() || !setCookieVar.isValid()){
            redirect(locationVar.toString(), QByteArray());
        }else{
            redirect(locationVar.toString(), setCookieVar.toByteArray());
        }
        return;
    }

    NetworkResponse response;
    response.statusCode = statusCode.toString();
    response.dataType = m_dataType;

    QList<QNetworkReply::RawHeaderPair> ls = m_reply->rawHeaderPairs();
    foreach(QNetworkReply::RawHeaderPair it, ls){
        response.headers.insert(it.first, it.second);
    }
    if(!m_cookieJar.isNull()){
        response.cookies = m_cookieJar->getCookieStrings();
    }

    response.body = QString::fromUtf8(m_reply->readAll());
    emit finished(m_callbackID, response);
}

void NetworkPrivate::onError(QNetworkReply::NetworkError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
    QString networkErrMsg = m_reply->errorString();
    QString msg = QString("%1 %2::%3")
        .arg(networkErrMsg.isEmpty()?"":networkErrMsg, metaEnum.name(), metaEnum.valueToKey(static_cast<int>(err)));
    setError(msg);
    emit error(m_callbackID, msg);
}




QJsonObject NetworkResponse::headerJson() const
{
  QJsonObject result;
  if(!headers.isEmpty()){
        QMapIterator<QString,QString> it(headers);
        while(it.hasNext()){
            it.next();
            result.insert(it.key(), QJsonValue(it.value()));
        }
  }
  return result;
}


QJsonArray NetworkResponse::cookieJson() const
{
    return QJsonArray::fromStringList(cookies);
}


QJsonObject NetworkResponse::bodyJson() const
{
    return QJsonDocument::fromJson(body.toUtf8()).object();
}

