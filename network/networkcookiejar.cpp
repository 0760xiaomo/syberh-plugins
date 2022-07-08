#include "networkcookiejar.h"

NetworkCookieJar::NetworkCookieJar(QObject *parent) : QNetworkCookieJar(parent)
{
}

NetworkCookieJar::~NetworkCookieJar() = default;


QList<QNetworkCookie> NetworkCookieJar::getCookies() const
{
    return allCookies();
}

QStringList NetworkCookieJar::getCookieStrings() const
{
    QList<QNetworkCookie> cookies = allCookies();
    QStringList list;
    foreach(auto cookie, cookies){
        list.push_back(cookie.toRawForm(QNetworkCookie::Full));
    }
    return list;
}

void NetworkCookieJar::setCookies(const QMap<QString,QString> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    QMapIterator<QString,QString> i(cookies);
    while(i.hasNext()){
        i.next();
        QNetworkCookie cookie(i.key().toUtf8(), i.value().toUtf8());
        insertCookie(cookie);
    }
}

void NetworkCookieJar::setCookies(const QList<QNetworkCookie> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    setAllCookies(cookies);
}

void NetworkCookieJar::updateCookies(const QMap<QString,QString> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    QMapIterator<QString,QString> i(cookies);
    while(i.hasNext()){
        i.next();
        QNetworkCookie cookie(i.key().toUtf8(), i.value().toUtf8());
        updateCookie(cookie);
    }
}


void NetworkCookieJar::updateCookies(const QList<QNetworkCookie> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    foreach(auto cookie, cookies){
        updateCookie(cookie);
    }
}
