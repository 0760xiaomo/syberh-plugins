#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "system_global.h"

class SYSTEMSHARED_EXPORT System : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    System();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
    // signal manager
    ExtensionSystem::SignalManager *s;

    /**
     * @brief aboutPhone 获取手机信息
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void  aboutPhone(const QString &callbackID, const QVariantMap &params);

     /**
     * @brief setDate 设置系统时间
     * @param callBackID 任务Id
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void  setDate(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief getResolution 获取手机分辨率
     * @param callBackID 获取手机分辨率的任务Id
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void  getResolution(const QString &callbackID, const QVariantMap &params);

   /**
    * @brief getCoreVersion 获取内核版本号
    * @param callBackID 获取内核版本号的任务Id
    * @param params 参数
    * @return 成功则无返回。
    *      失败则返回错误码。
    */
   void  getCoreVersion(const QString &callbackID, const QVariantMap &params);

   /**
    * @brief getSysVersionID 获取系统版本号
    * @param callBackID 获取内核版本号的任务Id
    * @param params 参数
    * @return 成功则无返回。
    *      失败则返回错误码。
    */
   void  getSysVersionID(const QString &callbackID, const QVariantMap &params);
  
  /**
     * @brief 截屏
     * @param callBackID 任务Id
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void  captureScreen(const QString &callbackID, const QVariantMap &params);
};

#endif // SYSTEM_H
