#ifndef UPDATE_P_H
#define UPDATE_P_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScopedPointer>

class Update;

class UpdatePrivate : public QObject
{
    Q_OBJECT
public:
    UpdatePrivate(Update *update);
    ~UpdatePrivate();

    void checkNewVersion(const QString &callbackID);
    void openAppStore();

private slots:
    void onFinished();
    void onError(QNetworkReply::NetworkError);

private:
    void clear();

    Update *p;
    long m_callbackID;
    QScopedPointer<QNetworkAccessManager> m_networkAccessManager;
    QScopedPointer<QNetworkReply> m_reply;
    bool m_checking;
};

#endif
