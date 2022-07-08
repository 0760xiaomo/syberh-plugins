#include "uploadcookiejar.h"

UploadCookieJar::UploadCookieJar(QObject *parent) : QNetworkCookieJar(parent)
{
}

UploadCookieJar::~UploadCookieJar() = default;


QList<QNetworkCookie> UploadCookieJar::getCookies() const
{
    return allCookies();
}

QStringList UploadCookieJar::getCookieStrings() const
{
    QList<QNetworkCookie> cookies = allCookies();
    QStringList list;
    foreach(auto cookie, cookies){
        list.push_back(cookie.toRawForm(QNetworkCookie::Full));
    }
    return list;
}

void UploadCookieJar::setCookies(const QMap<QString,QString> &cookies)
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

void UploadCookieJar::setCookies(const QList<QNetworkCookie> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    setAllCookies(cookies);
}

void UploadCookieJar::updateCookies(const QMap<QString,QString> &cookies)
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


void UploadCookieJar::updateCookies(const QList<QNetworkCookie> &cookies)
{
    if(cookies.isEmpty()){
        return;
    }
    foreach(auto cookie, cookies){
        updateCookie(cookie);
    }
}
