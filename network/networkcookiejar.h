#ifndef NETWORKCOOKIEJAR_H
#define NETWORKCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QMap>
#include <QList>

class NetworkCookieJar : public QNetworkCookieJar
{
public:
    NetworkCookieJar(QObject *parent = nullptr);
    virtual ~NetworkCookieJar();

    QList<QNetworkCookie> getCookies() const;
    QStringList getCookieStrings() const;
    void setCookies(const QMap<QString,QString> &cookies);
    void setCookies(const QList<QNetworkCookie> &cookies);

    void updateCookies(const QMap<QString,QString> &cookies);
    void updateCookies(const QList<QNetworkCookie> &cookies);
};

#endif  //NETWORKCOOKIEJAR_H
