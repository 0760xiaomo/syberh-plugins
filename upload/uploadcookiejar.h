#ifndef UPLOADCOOKIEJAR_H
#define UPLOADCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QMap>
#include <QList>

class UploadCookieJar : public QNetworkCookieJar
{
public:
    UploadCookieJar(QObject *parent = nullptr);
    virtual ~UploadCookieJar();

    QList<QNetworkCookie> getCookies() const;
    QStringList getCookieStrings() const;
    void setCookies(const QMap<QString,QString> &cookies);
    void setCookies(const QList<QNetworkCookie> &cookies);

    void updateCookies(const QMap<QString,QString> &cookies);
    void updateCookies(const QList<QNetworkCookie> &cookies);
};

#endif  //UPLOADCOOKIEJAR_H
