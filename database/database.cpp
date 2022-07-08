#include <QDebug>
#include <QtSql>
#include <QObject>
#include <QDebug>
#include <QException>

#include "database_p.h"
#include "database.h"
#include "helper.h"
#include "framework/common/errorinfo.h"


#define DB_CHECK(cond, callbackID, code, msg) \
    if(cond){ \
        signalManager()->failed(callbackID.toLong(), code, msg); \
        return; \
    }

#define DB_NCHECK(cond, callbackID, code, msg) \
    if(!cond){ \
        signalManager()->failed(callbackID.toLong(), code, msg); \
        return; \
    }


using namespace NativeSdk;


void Database::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;
    if (actionName == "query"){
        query(callbackID, params);
    }else if (actionName == "exec"){
        exec(callbackID, params);
    }else if (actionName == "databaseExists"){
        databaseExists(callbackID, params);
    }else if (actionName == "tableExists"){
        tableExists(callbackID, params);
    }
}

void Database::query(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << params;

    QString databaseName = params.value("database").toString();//数据库名
    QString sql = params.value("sql").toString();

    DB_CHECK(databaseName.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "数据库名不能为空")
    DB_CHECK(sql.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "sql不能为空")

    DatabasePrivate private_(databaseName);
    QString err;
    DB_NCHECK(private_.connect(&err), callbackID, ErrorInfo::SystemError, err)

    QVariant paramsVar = params.value("params");

    QJsonArray jsonArray;
    auto rowMapFunc = [&jsonArray](int rowNum, const QMap<QString,QVariant> &rowMap){
        Q_UNUSED(rowNum)
        QJsonObject jsonObj = QJsonObject::fromVariantMap(rowMap);
        jsonArray.append(jsonObj);
    };
    // 无参数sql
    if(!paramsVar.isValid()){
        private_.queryMap(sql, rowMapFunc, &err);
    }else if(paramsVar.canConvert<QVariantList>()){
        private_.queryMap(sql, paramsVar.toList(), rowMapFunc, &err);
    }else if(paramsVar.canConvert<QVariantMap>()){
        private_.queryMap(sql, paramsVar.toMap(), rowMapFunc, &err);
    }else if(paramsVar.canConvert<IndexMap>()){
        private_.queryMap(sql, paramsVar.value<IndexMap>(), rowMapFunc, &err);
    }else{
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "sql参数格式错误");
        return;
    }

    qDebug() << Q_FUNC_INFO << "result:" << jsonArray;
    signalManager()->success(callbackID.toLong(), QVariant(jsonArray));
}

void Database::exec(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << params;

    QString databaseName = params.value("database").toString();//数据库名
    QString sql = params.value("sql").toString();

    DB_CHECK(databaseName.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "数据库名不能为空")
    DB_CHECK(sql.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "sql不能为空")

    DatabasePrivate private_(databaseName);
    QString err;
    DB_NCHECK(private_.connect(&err), callbackID, ErrorInfo::SystemError, err)

    QVariant paramsVar = params.value("params");
    // 无参数sql
    if(!paramsVar.isValid()){
        DB_NCHECK(private_.exec(sql, &err), callbackID, ErrorInfo::SystemError, err)
        signalManager()->success(callbackID.toLong(), true);
        return;
    }

    if(paramsVar.canConvert<QVariantList>()){
        DB_NCHECK(private_.exec(sql, paramsVar.toList(), &err), callbackID, ErrorInfo::SystemError, err)
    }else if(paramsVar.canConvert<QVariantMap>()){
        DB_NCHECK(private_.exec(sql, paramsVar.toMap(), &err), callbackID, ErrorInfo::SystemError, err)
    }else{
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "sql参数格式错误");
        return;
    }
    signalManager()->success(callbackID.toLong(), true);
}

void Database::databaseExists(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << params;

    QString databaseName = params.value("database").toString();//数据库名
    DB_CHECK(databaseName.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "数据库名不能为空")

    DatabasePrivate private_(databaseName);
    bool isExists = private_.isDatabaseExists();
    signalManager()->success(callbackID.toLong(), isExists);
}

void Database::tableExists(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << params;

    QString databaseName = params.value("database").toString();//数据库名
    QString tableName = params.value("table").toString();

    DB_CHECK(databaseName.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "数据库名不能为空")
    DB_CHECK(tableName.isEmpty(), callbackID, ErrorInfo::InvalidParameter, "表名不能为空")

    DatabasePrivate private_(databaseName);
    QString err;
    DB_NCHECK(private_.connect(&err), callbackID, ErrorInfo::SystemError, err)

    bool isExists = private_.isTableExists(tableName, &err);
    if(!isExists){
        DB_NCHECK(err.isEmpty(), callbackID, ErrorInfo::SystemError, err)
    }
    signalManager()->success(callbackID.toLong(), isExists);
}
