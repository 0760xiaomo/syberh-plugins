#ifndef NEWWORK_P_H
#define NEWWORK_P_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QList>
#include <QScopedPointer>
#include <QJsonObject>
#include <QJsonArray>

class NetworkCookieJar;

struct NetworkResponse;


class NetworkPrivate : public QObject
{
    Q_OBJECT
public:
    enum Method {
        GET = 1,
        POST,
        PUT,
        PATCH,
        DELETE,
        HEAD,
        OPTIONS
    };
    Q_ENUM(Method)

    enum DataType {
        JSON = 1,
        TEXT
    };
    Q_ENUM(DataType)

    const static Method defaultMethod = GET;

    virtual ~NetworkPrivate();

    static NetworkPrivate* New(const QString &callbackID, QObject *parent = nullptr);

    inline NetworkPrivate* setUrl(const QString &url) { m_url = url; return this; }
    inline NetworkPrivate* setMethod(Method method) { m_method = method; return this; }
    inline NetworkPrivate* setMethod(const QString &method) { m_method = toMethod(method); return this; }
    NetworkPrivate* setCookies(const QVariantMap &cookies);
    NetworkPrivate* setCookies(const QString &cookies);
    inline NetworkPrivate* setHeaders(const QVariantMap &headers) { m_headers = headers; return this; }
    inline NetworkPrivate* setData(const QString &data) { m_data = data; return this; }
    NetworkPrivate* setData(const QMap<QString,QString> &data);
    NetworkPrivate* setData(const QMap<QString, QVariant> &data);

    inline NetworkPrivate* setDataType(DataType dataType) { m_dataType = dataType; return this; }
    inline NetworkPrivate* setDataType(const QString &dataType) { m_dataType = toDataType(dataType); return this; }

    inline NetworkPrivate* setAutoRedirect(bool autoRedirect) { m_autoRedirect = autoRedirect; return this; }

    void request();

    inline bool hasError() { return !m_error.isEmpty(); }
    QString& getError() { return m_error; }

signals:
    void finished(const QString &callbackID, const NetworkResponse &response);
    void error(const QString &callbackID, const QString &err);

private slots:
    void onFinished();
    void onError(QNetworkReply::NetworkError);

private:
    NetworkPrivate(const QString &callbackID, QObject *parent = nullptr);
    void redirect(const QString &url, const QByteArray &cookieString);

    inline void setError(const QString &err) { m_error = err; }

    bool setRequestUrl(QNetworkRequest &request);
    bool checkRequestMethod(QString *outMethod);
    bool setRequestHeaders(QNetworkRequest &request, const QVariantMap &headers);
    bool checkRequestData(QByteArray *outData);
    bool setRequestCookies(QNetworkRequest &request);

    Method toMethod(const QString &);
    DataType toDataType(const QString &);


    QScopedPointer<QNetworkAccessManager> m_manager;
    QScopedPointer<QNetworkReply> m_reply;
    QScopedPointer<NetworkCookieJar> m_cookieJar;
    QString m_callbackID;
    QString m_url;
    Method m_method = defaultMethod;
    QList<QNetworkCookie> m_cookies;
    QVariantMap m_headers;
    QString m_data;
    DataType m_dataType;
    QString m_error;

    bool m_autoRedirect = true;

    Q_DISABLE_COPY(NetworkPrivate);
};


struct NetworkResponse
{
    QString statusCode;
    QMap<QString,QString> headers;
    QStringList cookies;
    QString body;
    NetworkPrivate::DataType dataType;

    QJsonObject headerJson() const;
    QJsonArray cookieJson() const;
    QJsonObject bodyJson() const;
};


#endif  //NEWWORK_P_H
