#include <QDebug>
#include <QtSql>
#include <QObject>
#include <QDebug>
#include <QMapIterator>

#include "database_p.h"
#include "helper.h"
#include "framework/common/errorinfo.h"


#define DB_P_DRIVER_TYPE "QSQLITE"

#define DB_P_RETURN_ERROR(err, msg, errHandle) \
    *err = msg; \
    errHandle;

#define DB_P_CHECK(cond, err, msg, errHandle) \
    if(cond){ \
        DB_P_RETURN_ERROR(err, msg, errHandle) \
    }

#define DB_P_CHECK_NULLPTR(x, err, msg, errHandle) \
    if(x == nullptr){ \
        DB_P_RETURN_ERROR(err, msg, errHandle) \
    }

#define DB_P_CHECK_IS_EMPTY(x, err, msg, errHandle) \
    if(x.isEmpty()){ \
        DB_P_RETURN_ERROR(err, msg, errHandle) \
    }


#define DB_P_CHECK_SQL(sql, err, errHandle) \
    if(sql.isEmpty()){ \
        DB_P_RETURN_ERROR(err, "sql不能为空", errHandle) \
    }


#define DB_P_CHECK_PARAMS(params, err, errHandle) \
    if(params.isEmpty()){ \
        DB_P_RETURN_ERROR(err, "sql参数不能为空", errHandle) \
    }


#define DB_P_LAST_ERROR(var, err, errHandle) \
    *err = getLastError(var.lastError()); \
    errHandle;


#define DB_P_OPEN(conn, err, errHandle) \
    conn.setDatabaseName(fullDatabasePath()); \
    if(!open(err)) { \
        errHandle; \
    }


#define DB_P_QUERY_PREPARE(queryVar, conn, sql, err, errHandle) \
    QSqlQuery queryVar(conn); \
    if(!queryVar.prepare(sql)){ \
        DB_P_LAST_ERROR(queryVar, err, errHandle) \
    }


#define DB_P_PREPARE_BIND_MAP(queryVar, itVar, keyType, params) \
    QMapIterator<keyType, QVariant> itVar(params); \
    while (itVar.hasNext()) { \
        itVar.next(); \
        queryVar.bindValue(itVar.key(), itVar.value()); \
    }

#define DB_P_PREPARE_BIND_LIST(queryVar, params) \
    for(int i_=0; i_<params.size(); i_++){ \
        queryVar.bindValue(i_, params.at(i_)); \
    }


#define DB_P_EXEC(queryVar, err, errHandle) \
    if(!queryVar.exec()){ \
        DB_P_LAST_ERROR(queryVar, err, errHandle) \
    }


#define DB_P_EXEC_SQL(queryVar, sql, err, errHandle) \
    if(!queryVar.exec(sql)){ \
        DB_P_LAST_ERROR(queryVar, err, errHandle) \
    }



using namespace NativeSdk;

DatabasePrivate::DatabasePrivate(const QString &dbName, QObject *parent)
    : QObject(parent), m_dbName(dbName)
{
    m_dataDir = Helper::instance()->getDataRootPath();
}

DatabasePrivate::~DatabasePrivate()
{
    if(m_conn.isOpen()){
        m_conn.close();
    }
}

bool DatabasePrivate::connect(QString *err)
{
    return connect(QString(), err);
}

bool DatabasePrivate::connect(const QString &connectionName, QString *err)
{
    DB_P_CHECK(!isDirExists(m_dataDir, true), err, "未找到目录:" + m_dataDir, return false)

    if(connectionName.isEmpty()){
        if(QSqlDatabase::contains(QSqlDatabase::defaultConnection)){
            m_conn = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
        }else{
            m_conn = QSqlDatabase::addDatabase(DB_P_DRIVER_TYPE);
        }
    }else{
        if(QSqlDatabase::contains(connectionName)){
            m_conn = QSqlDatabase::database(connectionName, false);
        }else{
            m_conn = QSqlDatabase::addDatabase(DB_P_DRIVER_TYPE, connectionName);
        }
    }
    if(!m_conn.isValid()){
        DB_P_LAST_ERROR(m_conn, err, return false)
    }
    return true;
}


bool DatabasePrivate::query(const QString &sql, CellRecordHandleFunc cellHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    QSqlQuery query(m_conn);
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        for(int i = 0 ; i<rec.count(); ++i){
            cellHandle(rowNum, i, rec);
        }
        rowNum++;
    }
    return true;
}

bool DatabasePrivate::query(const QString &sql, const QVariantList &params, CellRecordHandleFunc cellHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_LIST(query, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        for(int i = 0 ; i<rec.count(); ++i){
            cellHandle(rowNum, i, rec);
        }
        rowNum++;
    }
    return true;
}


bool DatabasePrivate::query(const QString &sql, const IndexMap &params, CellRecordHandleFunc cellHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, int, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        for(int i = 0 ; i<rec.count(); ++i){
            cellHandle(rowNum, i, rec);
        }
        rowNum++;
    }
    return true;
}

bool DatabasePrivate::query(const QString &sql, const QVariantMap &params, CellRecordHandleFunc cellHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, QString, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        for(int i = 0 ; i<rec.count(); ++i){
            cellHandle(rowNum, i, rec);
        }
        rowNum++;
    }
    return true;
}

bool DatabasePrivate::queryMap(const QString &sql, RowMapHandleFunc rowHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    QSqlQuery query(m_conn);
    DB_P_EXEC_SQL(query, sql, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        QVariantMap rowMap;
        for(int i = 0 ; i<rec.count(); ++i){
            rowMap.insert(rec.fieldName(i), rec.value(i));
        }
        rowHandle(rowNum, rowMap);
        rowNum++;
    }
    return true;
}


bool DatabasePrivate::queryMap(const QString &sql, const QVariantList &params, RowMapHandleFunc rowHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_LIST(query, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        QVariantMap rowMap;
        for(int i = 0 ; i<rec.count(); ++i){
            rowMap.insert(rec.fieldName(i), rec.value(i));
        }
        rowHandle(rowNum, rowMap);
        rowNum++;
    }
    return true;
}


bool DatabasePrivate::queryMap(const QString &sql, const IndexMap &params, RowMapHandleFunc rowHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, int, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        QVariantMap rowMap;
        for(int i = 0 ; i<rec.count(); ++i){
            rowMap.insert(rec.fieldName(i), rec.value(i));
        }
        rowHandle(rowNum, rowMap);
        rowNum++;
    }
    return true;
}

bool DatabasePrivate::queryMap(const QString &sql, const QVariantMap &params, RowMapHandleFunc rowHandle, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, QString, params)
    DB_P_EXEC(query, err, return false)

    int rowNum = 0;
    while(query.next()) {
        QSqlRecord rec = query.record();
        QVariantMap rowMap;
        for(int i = 0 ; i<rec.count(); ++i){
            rowMap.insert(rec.fieldName(i), rec.value(i));
        }
        rowHandle(rowNum, rowMap);
        rowNum++;
    }
    return true;
}


bool DatabasePrivate::exec(const QString &sql, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)

    DB_P_OPEN(m_conn, err, return false)

    QSqlQuery query(m_conn);
    DB_P_EXEC_SQL(query, sql, err, return false)
    return true;
}


bool DatabasePrivate::exec(const QString &sql, const QVariantList &params, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_LIST(query, params)
    DB_P_EXEC(query, err, return false)
    return true;
}


bool DatabasePrivate::exec(const QString &sql, const IndexMap &params, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, int, params)
    DB_P_EXEC(query, err, return false)
    return true;
}

bool DatabasePrivate::exec(const QString &sql, const QVariantMap &params, QString *err)
{
    DB_P_CHECK_SQL(sql, err, return false)
    DB_P_CHECK_PARAMS(params, err, return false)

    DB_P_OPEN(m_conn, err, return false)
    DB_P_QUERY_PREPARE(query, m_conn, sql, err, return false)
    DB_P_PREPARE_BIND_MAP(query, it, QString, params)
    DB_P_EXEC(query, err, return false)
    return true;
}

bool DatabasePrivate::execWithTransaction(TransactionHandleFunc transactionFunc, QString *err)
{
    DB_P_CHECK_NULLPTR(transactionFunc, err, "事务函数不能为空", return false)

    DB_P_OPEN(m_conn, err, return false)
    QSqlQuery query(m_conn);
    if(!m_conn.transaction()){
        DB_P_LAST_ERROR(m_conn, err, return false)
    }
    bool needCommit = transactionFunc(query);
    if(needCommit) {
        if(!m_conn.commit()){
            DB_P_LAST_ERROR(m_conn, err, return false)
        }
    }else{
        if(!m_conn.rollback()){
            DB_P_LAST_ERROR(m_conn, err, return false)
        }
    }
    return true;
}


bool DatabasePrivate::isTableExists(const QString &tableName, QString *err)
{
    DB_P_CHECK_IS_EMPTY(tableName, err, "表名不能为空", return false)

    if (!isDatabaseExists()) {
        return false;
    }

    DB_P_OPEN(m_conn, err, return false)
    // 查询 sqlite_master 表
    DB_P_QUERY_PREPARE(query, m_conn, "select * from sqlite_master where type = 'table' and name = ?", err, return false)

    query.addBindValue(tableName);

    DB_P_EXEC(query, err, return false)

    // 指针指向第一个结果，false： 表不存在
    if (!query.first()) {
        return false;
    }
    QSqlRecord rec = query.record();
    // 表名无效，说明表不存在
    if (!rec.value("name").isValid()) {
        return false;
    }
    return true;
}

bool DatabasePrivate::isDirExists(const QString &dirPath, bool createIfNotExists)
{
    if(dirPath.isEmpty()){
        return false;
    }
    QDir dir(dirPath);
    if(dir.exists()){
        return true;
    }
    if(!createIfNotExists){
        return false;
    }
    if(!dir.mkpath(dirPath)){
        return false;
    }
    return true;
}

bool DatabasePrivate::isDatabaseExists() {
    QString dbPath = fullDatabasePath();
    //判断数据库是否存在
    QFileInfo dbFileInfo(dbPath);
    if(!dbFileInfo.exists() || !dbFileInfo.isFile()){
        return false;
    }
    return true;
}

bool DatabasePrivate::open(QString *err)
{
    if(!m_conn.isValid()){
        DB_P_RETURN_ERROR(err, "无效的驱动" + m_conn.driverName(), return false)
    }
    if(m_conn.isOpen()){
        return true;
    }
    m_conn.setDatabaseName(fullDatabasePath());
    if(!m_conn.open()) {
        DB_P_LAST_ERROR(m_conn, err, return false)
    }
    return true;
}
