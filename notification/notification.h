#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "notification_global.h"

class NOTIFICATIONSHARED_EXPORT Notification: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Notification();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
    /**
     * @brief badgeShow 角标显示
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void badgeShow(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief sendNotification 消息发送
     * @param params 参数
     * @return 成功则消息id。
     *      失败则返回错误码。
     */
    void sendNotification(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief removeAllNotifications 删除消息
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void removeAllNotifications(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief removeNotification 删除指定消息
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void removeNotification(const QString &callbackID, const QVariantMap &params);

};

#endif // NOTIFICATION_H
