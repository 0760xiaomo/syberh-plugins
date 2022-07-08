#ifndef CELLULAR_H
#define CELLULAR_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "cellular_global.h"

class CELLULARSHARED_EXPORT Cellular: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Cellular();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
    /**
     * @brief aboutPhone 设置数据网络状态为关闭
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数
     * @return 设置成功返回 true
     * @return 设置失败返回 false
     */
    void  networkDisabled(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief aboutPhone 设置数据网络状态为启用
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数
     * @return 设置成功返回 true
     * @return 设置失败返回 false
     */
    void  networkEnabled(const QString &callbackID, const QVariantMap &params);
};


#endif // CELLULAR_H
