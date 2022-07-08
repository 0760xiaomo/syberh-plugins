#ifndef VIBRATOR_H
#define VIBRATOR_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "vibrator_global.h"

#include "cngfmanager.h"
#include "csystemprofilemanager.h"

class VIBRATORSHARED_EXPORT Vibrator: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Vibrator();
    ~Vibrator();

    void invokeInitialize();
    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

private :
    long globalCallbackID;
    static bool vibratorState;

    /**
     * @brief changeCameraImagePath 触发震动
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void vibrate(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief changeCameraImagePath 查询震动设置
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void vibrateInfo(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief changeCameraImagePath 设置物理按键是否震动。
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setTouchVibrationSoundEnabled(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief changeCameraImagePath 设置键盘按键震动是否开启。
     * @param params 参数
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setVibratingEnabled(const QString &callbackID, const QVariantMap &params);

    CNgfManager *client;
    CSystemProfileManager *profile;

public slots:
    void eventFailed(quint32 eventId);
    void eventCompleted(quint32 eventId);
    void eventPlaying(quint32 eventId);

};

#endif // VIBRATOR_H
