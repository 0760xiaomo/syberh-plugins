#ifndef HISTORYDATA_H
#define HISTORYDATA_H

#include <QObject>
#include <QtPlugin>
#include <QSqlDatabase>

class HistoryData : public QObject
{
    Q_OBJECT

public:
    HistoryData();

    void insertMetadata(const QString &path, int size, int duration, const QString &created);
    void removeMetadata(const QString &path);
    void updateMetadata(const QString &path, int size, int duration);
    QJsonArray selectMetadata();

private :
    QSqlDatabase myConnection;

    bool createConnection();
    bool checkOrCreateDir(const QString &path);
    QString formatTime(int duration);
};

#endif // HISTORYDATA_H
