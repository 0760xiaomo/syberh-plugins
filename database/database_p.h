#ifndef DATABASE_P_H
#define DATABASE_P_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMap>
#include <QList>
#include <functional>

typedef QMap<int,QVariant> IndexMap;

typedef std::function<void (int rowNum, int colNum, const QSqlRecord &rowRecord)> CellRecordHandleFunc;

typedef std::function<void (int rowNum, const QMap<QString,QVariant> &rowMap)> RowMapHandleFunc;

typedef std::function<bool (const QSqlQuery &query)> TransactionHandleFunc;


class DatabasePrivate : public QObject
{
    Q_OBJECT
public:
    DatabasePrivate(const QString &dbName, QObject *parent = nullptr);
    virtual ~DatabasePrivate();

    bool connect(QString *err);

    bool connect(const QString &connectionName, QString *err);


    bool query(const QString &sql, CellRecordHandleFunc cellHandle, QString *err);

    bool query(const QString &sql, const QVariantList &params, CellRecordHandleFunc cellHandle, QString *err);

    bool query(const QString &sql, const IndexMap &params, CellRecordHandleFunc cellHandle, QString *err);

    bool query(const QString &sql, const QVariantMap &params, CellRecordHandleFunc cellHandle, QString *err);

    bool queryMap(const QString &sql, RowMapHandleFunc rowHandle, QString *err);

    bool queryMap(const QString &sql, const QVariantList &params, RowMapHandleFunc rowHandle, QString *err);

    bool queryMap(const QString &sql, const IndexMap &params, RowMapHandleFunc rowHandle, QString *err);

    bool queryMap(const QString &sql, const QVariantMap &params, RowMapHandleFunc rowHandle, QString *err);


    bool exec(const QString &sql, QString *err);

    bool exec(const QString &sql, const QVariantList &params, QString *err);

    bool exec(const QString &sql, const IndexMap &params, QString *err);

    bool exec(const QString &sql, const QVariantMap &params, QString *err);

    bool execWithTransaction(TransactionHandleFunc transactionFunc, QString *err);


    bool isTableExists(const QString &tableName, QString *err);

    bool isDirExists(const QString &dirPath, bool createIfNotExists = false);

    bool isDatabaseExists();

private:
    inline QString fullDatabasePath() { return QString("%1/%2.sqlite").arg(m_dataDir, m_dbName); }
    inline QString getLastError(const QSqlError &sqlErr) { return sqlErr.isValid() ? sqlErr.text() : QString(); }

    bool open(QString *err);

    QSqlDatabase m_conn;
    // app data存放位置
    QString m_dataDir;
    QString m_dbName;
};

#endif // DATABASE_P_H
