#ifndef UPLOAD_P_H
#define UPLOAD_P_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QPair>
#include <QScopedPointer>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonObject>

class UploadCookieJar;

class UploadResponse
{
public:
    QString statusCode;
    QMap<QString,QString> header;
    QStringList cookie;
    QString body;
    qint64 bytesSent;
    qint64 bytesTotal;

    QJsonValue toJson() const;
};


class UploadPrivate : public QObject
{
    Q_OBJECT
public:
    UploadPrivate(const QString &callbackID, QObject *parent = nullptr);
    virtual ~UploadPrivate();

    // connect timeout ms
    inline void setTimeout(int timeout) { m_timeout = timeout; }
    inline void setUrl(const QString &url) { m_url = url; }
    void setCookies(const QVariantMap &cookies);
    void setCookies(const QString &cookies);
    inline void setHeaders(const QVariantMap &headers) { m_headers = headers; }
    void setForm(const QVariantMap &data);
    void setFile(const QString &name, const QString &filePath);

    void upload();
    void abort();

    inline qint64 bytesSent() const { return m_bytesSent; }
    inline qint64 bytesTotal() const { return m_bytesTotal; }

    inline bool hasError() { return !m_error.isEmpty(); }
    QString& getError() { return m_error; }

signals:
    void started(const QString &callbackID);
    void finished(const QString &callbackID, const UploadResponse &response);
    void uploadProcess(const QString &callbackID, qint64 bytesSent, qint64 bytesTotal);
    void error(const QString &callbackID, const QString &err);

private slots:
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onFinished();
    void onError(QNetworkReply::NetworkError);

private:
    Q_DISABLE_COPY(UploadPrivate);

    inline void setError(const QString &err) { m_error = err; }

    void putUrl(QNetworkRequest &request);
    void putHeader(QNetworkRequest &request);
    void putCookie(QNetworkRequest &request);
    void putFormData(QHttpMultiPart *multiPart, QNetworkRequest &request);

    QScopedPointer<QNetworkAccessManager> m_manager;
    QScopedPointer<QNetworkReply> m_reply;
    QScopedPointer<UploadCookieJar> m_cookieJar;

    qint64 m_bytesSent;
    qint64 m_bytesTotal;

    QString m_error;
    QString m_callbackID;
    int m_timeout; //ms
    QString m_url;
    QList<QNetworkCookie> m_cookies;
    QVariantMap m_headers;
    QVariantMap m_form;
    QPair<QString, QString> m_file;
};

#endif
