#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "brightness_global.h"

class BRIGHTNESSSHARED_EXPORT Brightness : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Brightness();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :

    /**
     * @brief maximumBrightness 获取系统屏幕亮度信息。
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数
     * @return 成功则返回系统屏幕亮度信息。
     *      失败则返回错误码。
     */
    void brightnessInfo(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief setAdaptiveDimmingEnabled 设置自适应调光开关。
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数 true 使能该功能，false 关闭该功能
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setAdaptiveDimmingEnabled(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief setAmbientLightSensorEnabled 开关环境光感器。
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数 true 使能该功能，false 关闭该功能。
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setAmbientLightSensorEnabled(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief setBlankTimeout 同setDimTimeout。
     */
    void setBlankTimeout(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief setBrightness 设置屏幕亮度。
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数 要设置的亮度值。
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setBrightness(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief :setDimTimeout 设置息屏时长。
     * @param callBackID 获取手机信息的任务Id
     * @param params 参数 息屏时长，单位秒，仅支持固定的时间。
     * @return 成功则无返回。
     *      失败则返回错误码。
     */
    void setDimTimeout(const QString &callbackID, const QVariantMap &params);
};

#endif // BRIGHTNESS_H
