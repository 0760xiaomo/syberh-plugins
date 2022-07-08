#include "upload_p.h"
#include "uploadcookiejar.h"
#include "util/validator.h"

#include <QNetworkConfiguration>
#include <QHttpPart>
#include <QFile>
#include <QMetaEnum>
#include <QJsonArray>
#include <QDebug>


#define UL_CONN_TIMEOUT 10000

#define UL_EQUALS(s1, s2) (QString::compare(s1, s2, Qt::CaseInsensitive) == 0)


// UploadResponse
QJsonValue UploadResponse::toJson() const
{
    QJsonObject json;
    json.insert("statusCode", statusCode);

    QJsonObject headerJson;
    if(!header.isEmpty()){
        QMapIterator<QString,QString> it(header);
        while(it.hasNext()){
            it.next();
            headerJson.insert(it.key(), QJsonValue(it.value()));
        }
    }
    json.insert("header", headerJson);
    json.insert("cookie", QJsonArray::fromStringList(cookie));
    json.insert("body", body);
    json.insert("bytesSent", bytesSent);
    json.insert("bytesTotal", bytesTotal);
    return json;
}



// UploadPrivate
UploadPrivate::UploadPrivate(const QString &callbackID, QObject *parent) : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_cookieJar(new UploadCookieJar(this))
    , m_callbackID(callbackID)
    , m_timeout(0)
{
}

UploadPrivate::~UploadPrivate()
{
    if(!m_reply.isNull()){
        QObject::disconnect(m_reply.data());
    }
}


void UploadPrivate::setCookies(const QVariantMap &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    QMapIterator<QString,QVariant> it(cookies);
    while(it.hasNext()){
        auto item = it.next();
        m_cookies.push_back(QNetworkCookie(item.key().toUtf8(), item.value().toByteArray()));
    }
}

void UploadPrivate::setCookies(const QString &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    m_cookies = QNetworkCookie::parseCookies(cookies.toUtf8());
}

void UploadPrivate::setForm(const QVariantMap &data)
{
    if(data.isEmpty()){
        return;
    }
    m_form = data;
}

void UploadPrivate::setFile(const QString &name, const QString &filePath)
{
    if(name.isEmpty() || filePath.isEmpty()){
        return;
    }
    m_file.first = name;
    m_file.second = filePath;
}

void UploadPrivate::putUrl(QNetworkRequest &request)
{
    //检查url
    if(m_url.isEmpty()){
        setError("URL为空");
        return;
    }
    if (!NativeSdk::Validator::isHttpUrl(m_url)) {
        setError("URL无效");
        return;
    }
    if (m_url.startsWith("https", Qt::CaseInsensitive)) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::UnknownProtocol);
        request.setSslConfiguration(config);
    }
    request.setUrl(m_url);
}

void UploadPrivate::putHeader(QNetworkRequest &request)
{
    if(m_headers.isEmpty()){
        return;
    }
    QVariantMap::ConstIterator it = m_headers.constBegin();
    while (it != m_headers.constEnd()) {
        QString headerName = it.key();
        if(UL_EQUALS(headerName, "Content-Type") || UL_EQUALS(headerName, "ContentType")){
            continue;
        }
        QVariant value = it.value();
        if(value.canConvert<QString>()){
            request.setRawHeader(headerName.toUtf8(), value.toString().toUtf8());
        }else{
            request.setRawHeader(headerName.toUtf8(), value.toByteArray());
        }
        it++;
    }
}

void UploadPrivate::putCookie(QNetworkRequest &request)
{
    if(m_cookies.isEmpty()){
        return;
    }
    m_cookieJar->setCookies(m_cookies);
    m_manager->setCookieJar(m_cookieJar.data());

    QVariant var;
    var.setValue(m_cookies);
    request.setHeader(QNetworkRequest::CookieHeader, var);
}

void UploadPrivate::putFormData(QHttpMultiPart *multiPart, QNetworkRequest &request)
{
    Q_UNUSED(request)

    // 设置formData 额外参数
    if(!m_form.isEmpty()){
        QVariantMap::ConstIterator item = m_form.constBegin();
        while (item != m_form.constEnd()){
            QHttpPart textPart;
            textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + item.key() + "\""));
            QVariant value = item.value();
            if(value.canConvert<QString>()){
                textPart.setBody(value.toString().toUtf8());
            }else{
                textPart.setBody(value.toByteArray());
            }
            multiPart->append(textPart);
            item++;
        }
    }
    QString filePath = m_file.second;
    if(filePath.isEmpty() || !QFile::exists(filePath)){
        return;
    }
    // 若文件域未设置name, 则使用file作为name
    if(m_file.first.isEmpty()){
        m_file.first = "file";
    }

    QHttpPart filePart;
    if(m_headers.contains("Content-Type")){
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, m_headers.value("Content-Type"));
    }else if(m_headers.contains("ContentType")){
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, m_headers.value("ContentType"));
    }else{
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    }

    QFile *file = new QFile(filePath);
    if(!file->open(QIODevice::ReadOnly)){
        file->deleteLater();
        setError("无法打开文件");
        return;
    }
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
            QVariant("form-data; name=\"" + m_file.first + "\"; filename=\"" + file->fileName() + "\""));

    filePart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(filePart);
}

void UploadPrivate::upload()
{
    QNetworkRequest request;
    putUrl(request);
    putHeader(request);
    putCookie(request);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    putFormData(multiPart, request);

    if(hasError()){
        multiPart->deleteLater();
        return;
    }

    //timeout
    QNetworkConfiguration configure = m_manager->activeConfiguration();
    configure.setConnectTimeout(m_timeout>0 ? m_timeout : UL_CONN_TIMEOUT);
    m_manager->setConfiguration(configure);

    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::uploadProgress, this, &UploadPrivate::onUploadProgress);
    connect(reply, &QNetworkReply::finished, this, &UploadPrivate::onFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));

    m_reply.reset(reply);

    emit started(m_callbackID);
}

void UploadPrivate::abort()
{
    if(m_reply.isNull()){
        return;
    }
    disconnect(this);
    m_reply->abort();
    m_reply.reset();
}

void UploadPrivate::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if(bytesSent == bytesTotal && bytesTotal == 0){
        return;
    }
    m_bytesSent = bytesSent;
    m_bytesTotal = bytesTotal;
    emit uploadProcess(m_callbackID, bytesSent, bytesTotal);
}

void UploadPrivate::onFinished()
{
    if(m_reply.isNull()){
        return;
    }
    QVariant statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qDebug() << Q_FUNC_INFO << "statusCode:" << statusCode;

    if(m_reply->error() != QNetworkReply::NoError){
        qDebug() << Q_FUNC_INFO << "statusCode:" << statusCode << " error:" << m_reply->error();
        onError(m_reply->error());
        return;
    }

    UploadResponse response;
    response.statusCode = statusCode.toString();

    QList<QNetworkReply::RawHeaderPair> ls = m_reply->rawHeaderPairs();
    foreach(QNetworkReply::RawHeaderPair it, ls){
        response.header.insert(it.first, it.second);
    }
    if(!m_cookieJar.isNull()){
        response.cookie = m_cookieJar->getCookieStrings();
    }
    response.body = QString::fromUtf8(m_reply->readAll());
    response.bytesSent = m_bytesSent;
    response.bytesTotal = m_bytesTotal;
    emit finished(m_callbackID, response);
}

void UploadPrivate::onError(QNetworkReply::NetworkError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
    QString networkErrMsg = m_reply->errorString();
    QString msg = QString("%1 %2::%3")
        .arg(networkErrMsg.isEmpty()?"":networkErrMsg, metaEnum.name(), metaEnum.valueToKey(static_cast<int>(err)));

    qDebug() << Q_FUNC_INFO << msg;

    setError(msg);
    emit error(m_callbackID, msg);
}
