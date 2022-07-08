#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtPlugin>
#include <QSqlQueryModel>

#include "iplugin/iplugin.h"
#include "database_global.h"

class DATABASESHARED_EXPORT Database : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

    /**
     * @brief query 执行查询sql
     *
     * @param params 参数
     * @return 成功则返回数据。
     *      失败则返回错误码。
     */
    void query(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief execute 执行操作sql
     * @param params 参数
     * @return 成功则返回true。
     *      失败则返回错误码。
     */
    void exec(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief isDatabaseExists 判断数据库是否存在
     * @param params 参数
     * @return 成功则返回true。
     *      失败则返回错误码。
     */
    void databaseExists(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief isTableExists 判断表是否存在
     * @param params 参数
     * @return 成功则返回true。
     *      失败则返回错误码。
     */
    void tableExists(const QString &callbackID, const QVariantMap &params);

private:
    // app data存放位置
    QString dataDir;
};

#endif // DATABASE_H
