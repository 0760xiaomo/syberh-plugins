#include <QDebug>
#include <QDebug>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <cdisplaysettings.h>
#include <climits>

#include "brightness.h"
#include "framework/common/errorinfo.h"


#define BR_DIM_15S 15
#define BR_DIM_30S 30
#define BR_DIM_1M 60
#define BR_DIM_5M 300
#define BR_DIM_10M 600
#define BR_DIM_INFINITY -1

#define BR_TO_STR(x) #x
#define BR_DIM_TO_STR(x) BR_TO_STR(x)

#define BR_CHECK_DIMTIMEOUT(t) \
    (t==BR_DIM_15S || t==BR_DIM_30S || t==BR_DIM_1M || t==BR_DIM_5M || t==BR_DIM_10M || t==BR_DIM_INFINITY)

#define BR_FAIL_MESSAGE \
    QString("息屏时长设置错误，仅支持: %1/%2/%3/%4/%5 秒") \
            .arg(BR_DIM_TO_STR(BR_DIM_15S), BR_DIM_TO_STR(BR_DIM_30S), \
                 BR_DIM_TO_STR(BR_DIM_1M), BR_DIM_TO_STR(BR_DIM_5M), BR_DIM_TO_STR(BR_DIM_10M))



using namespace NativeSdk;

Brightness::Brightness()
{
}


void Brightness::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName=="brightnessInfo"){
        brightnessInfo(callbackID, params);
    }else if (actionName=="setAdaptiveDimmingEnabled"){
        setAdaptiveDimmingEnabled(callbackID, params);
    }else if (actionName=="setAmbientLightSensorEnabled"){
        setAmbientLightSensorEnabled(callbackID, params);
    }else if (actionName=="setBlankTimeout"){
        setBlankTimeout(callbackID, params);
    }else if (actionName=="setBrightness"){
        setBrightness(callbackID, params);
    }else if (actionName=="setDimTimeout"){
        setDimTimeout(callbackID, params);
    }
}


void Brightness::brightnessInfo(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "callbackID" << callbackID << ", params" << params << endl;
    CSystemDisplaySettings display;

    int max = display.maximumBrightness();
    bool adaptiveDimmingEnabled = display.adaptiveDimmingEnabled();
    bool ambientLightSensorEnabled = display.ambientLightSensorEnabled();

    int blankTime = display.blankTimeout();
    int brightness = display.brightness();
    int dimTimeout = display.dimTimeout();

    QJsonObject json;
    json.insert("maximumBrightness", max);
    json.insert("blankTime", blankTime);
    json.insert("brightness", brightness);
    json.insert("dimTimeout", dimTimeout);
    json.insert("ambientLightSensorEnabled", ambientLightSensorEnabled);
    json.insert("adaptiveDimmingEnabled", adaptiveDimmingEnabled);

    qDebug() << Q_FUNC_INFO << "json:" << json << endl;

    signalManager()->success(callbackID.toLong(), json);
}

void Brightness::setAdaptiveDimmingEnabled(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "callbackID" << callbackID << ", params" << params << endl;

    const QVariant stateVar = params.value("state");
    if(!stateVar.isValid()){
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "未设置控制参数");
        return;
    }
    QString state = stateVar.toString();
    bool enable = false;
    if(state == "0"){
        enable = false;
    }else if(state == "1"){
        enable = true;
    }else{
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "无效的控制参数");
        return;
    }

    CSystemDisplaySettings display;
    display.setAdaptiveDimmingEnabled(enable);

    QJsonObject json;
    json.insert("result", true);
    qDebug() << Q_FUNC_INFO << "json:" << json << endl;
    signalManager()->success(callbackID.toLong(), json);
}

void Brightness::setAmbientLightSensorEnabled(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "callbackID" << callbackID << ", params" << params << endl;

    const QVariant stateVar = params.value("state");
    if(!stateVar.isValid()){
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "未设置控制参数");
        return;
    }
    QString state = stateVar.toString();
    bool enable;
    if(state == "0"){
        enable = false;
    }else if(state == "1"){
        enable = true;
    }else{
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "无效的控制参数");
        return;
    }

    CSystemDisplaySettings display;
    display.setAmbientLightSensorEnabled(enable);

    QJsonObject json;
    json.insert("result", true);
    qDebug() << Q_FUNC_INFO << "json:" << json << endl;
    signalManager()->success(callbackID.toLong(), json);
}

void Brightness::setBlankTimeout(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "[obsoleted] invoke to setDimTimeout";
    setDimTimeout(callbackID, params);
}

void Brightness::setBrightness(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "callbackID" << callbackID << ", params" << params << endl;

    const QVariant brightnessVar = params.value("brightness");
    if(!brightnessVar.isValid() || !brightnessVar.canConvert<int>()){
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "未设置屏幕亮度");
        return;
    }

    int brightness = brightnessVar.toInt();
    if(brightness < 0 || brightness > 100){
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "无效的屏幕亮度值");
        return;
    }

    CSystemDisplaySettings display;
    display.setBrightness(brightness);

    QJsonObject json;
    json.insert("result", true);
    qDebug() << Q_FUNC_INFO << "json:" << json << endl;
    signalManager()->success(callbackID.toLong(), json);
}

void Brightness::setDimTimeout(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "callbackID" << callbackID << ", params" << params << endl;

    const QVariant timeoutVar = params.value("timeout");
    if(!timeoutVar.isValid() || !timeoutVar.canConvert<int>()){
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "未设置息屏时长");
        return;
    }

    int timeout = timeoutVar.toInt();

    if(!BR_CHECK_DIMTIMEOUT(timeout)){
        QString failMessage = BR_FAIL_MESSAGE;
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, failMessage);
        return;
    }
    if(timeout == BR_DIM_INFINITY){
        timeout = INT_MAX;
    }

    CSystemDisplaySettings display;
    // 从暗屏至灭屏的时间（秒）
    int blankTime = display.blankTimeout();
    // 计算从正常至暗屏的时间（秒），这个时间才是传入setDimTimeout接口的时间
    timeout -= blankTime;

    display.setDimTimeout(timeout);

    QJsonObject json;
    json.insert("result", true);
    qDebug() << Q_FUNC_INFO << "json:" << json << endl;
    signalManager()->success(callbackID.toLong(), json);
}

