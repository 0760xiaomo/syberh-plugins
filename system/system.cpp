#include "system.h"
#include <QDebug>
#include <cosinfo.h>
#include <csystemdeviceinfo.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <qscreen.h>
#include <qguiapplication.h>
#include <QSize>
#include <QTimeZone>
#include <QLocale>
#include <QDateTime>
#include <ctime.h>
#include <QDBusConnection>
#include <QDBusMessage>



static const QString COMPOSITOR_SERVICE_NAME = "com.syberos.compositor";
static const QString COMPOSITOR_OBJECT_PATH = "/com/syberos/compositor";
static const QString COMPOSITOR_INTERFACE_NAME = "com.syberos.compositor.CompositorInterface";

System::System()
{
}


void System::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    Q_UNUSED(actionName);

    if (actionName == "aboutPhone") {
        aboutPhone(callbackID, params);
    }else if (actionName == "setDate") {
        setDate(callbackID, params);
    }else if (actionName == "captureScreen") {
        captureScreen(callbackID, params);
    }
}


void System::aboutPhone(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(callbackID);
    Q_UNUSED(params);
    int modem = 0;

    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << ", params: " << params << endl;

    CSystemDeviceInfo deviceInfo;
    QJsonValue deviceId = QJsonValue::fromVariant(deviceInfo.uniqueDeviceId());
    QJsonValue name = QJsonValue::fromVariant(deviceInfo.productName());
    QJsonValue imei = QJsonValue::fromVariant(deviceInfo.imei(modem));
    QJsonArray simcardNumberJsonArr = QJsonArray::fromStringList(deviceInfo.simcardNumber());
    QJsonArray imsisJsonArr = QJsonArray::fromStringList(deviceInfo.imsis());
    COsInfo info;
    QJsonValue kernelVersion = QJsonValue::fromVariant(info.kernelVersion());
    QJsonValue osVersion = QJsonValue::fromVariant(info.osVersion());
    QJsonValue softwareVersion = QJsonValue::fromVariant(info.softwareVersion());
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect mm = screen->availableGeometry() ;
    int screenWidth = mm.width();
    int screenHeight = mm.height();
    qreal pixelRatio = screen->devicePixelRatio();//???????????????
    QLocale locale;
    QJsonValue language = QJsonValue::fromVariant(QLocale::languageToString(locale.language()));//??????????????????
    QJsonValue region = locale.countryToString(locale.country());//??????????????????

    QJsonObject jsonObject;
    jsonObject.insert("deviceId", deviceId);
    jsonObject.insert("brand", "");//????????????
    jsonObject.insert("manufacturer", "");//???????????????
    jsonObject.insert("model", "");//????????????
    jsonObject.insert("name", name);//????????????
    jsonObject.insert("imei", imei);//???????????????????????????
    jsonObject.insert("simCardNumbers", simcardNumberJsonArr);//????????????????????????????????????
    jsonObject.insert("imsis", imsisJsonArr);//??????????????????????????????????????????????????????
    jsonObject.insert("osType", "Syber");//??????????????????
    jsonObject.insert("osVersionName", softwareVersion);//????????????????????????
    jsonObject.insert("osVersion", osVersion);//?????????????????????
    jsonObject.insert("platformVersionName", "");//????????????????????????
    jsonObject.insert("platformVersionCode", "");//?????????????????????
    jsonObject.insert("kernelVersion", kernelVersion);//???????????????
    jsonObject.insert("screenWidth", screenWidth);//?????????
    jsonObject.insert("screenHeight", screenHeight);//?????????
    jsonObject.insert("windowWidth", "");//?????????????????????
    jsonObject.insert("windowHeight", "");//?????????????????????
    jsonObject.insert("statusBarHeight", "");//?????????????????? ???????????????
    jsonObject.insert("pixelRatio", pixelRatio);//???????????????
    jsonObject.insert("language", language);//????????????
    jsonObject.insert("region", region);//????????????

    QJsonValue jsonObjectValue = QJsonValue::fromVariant(jsonObject);

    qDebug() << Q_FUNC_INFO << "jsonObject:" << jsonObject << ", jsonObjectValue: " << jsonObjectValue << endl;

    signalManager()->success(callbackID.toLong(), QVariant(jsonObject));
}

void System::setDate(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(callbackID);
    Q_UNUSED(params);

    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << ", params: " << params << endl;
     QString date = params.value("date").toString();

    QDateTime dt = QDateTime::fromString(date, "yyyy-MM-dd hh:mm:ss");
    qDebug() << "time " << dt.toString();

    CTime time;
    time.setTime(dt);

    QJsonObject jsonObject;
    jsonObject.insert("date", date);

    QJsonValue jsonObjectValue = QJsonValue::fromVariant(jsonObject);

    qDebug() << Q_FUNC_INFO << "jsonObject:" << jsonObject << ", jsonObjectValue: " << jsonObjectValue << endl;

    signalManager()->success(callbackID.toLong(), QVariant(jsonObject));
}

void System::captureScreen(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(callbackID);
    Q_UNUSED(params);

    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << ", params: " << params << endl;

    QDBusMessage dbusMessage = QDBusMessage::createMethodCall(COMPOSITOR_SERVICE_NAME,
                                                              COMPOSITOR_OBJECT_PATH,
                                                              COMPOSITOR_INTERFACE_NAME,
                                                              "captureScreen");
    qDebug() << QDBusConnection::systemBus().send(dbusMessage);

    QJsonObject jsonObject;
    jsonObject.insert("success", true);
    QJsonValue jsonObjectValue = QJsonValue::fromVariant(jsonObject);

    signalManager()->success(callbackID.toLong(), QVariant(jsonObject));
}


void System::getResolution(const QString &callbackID, const QVariantMap &params){

    Q_UNUSED(params);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect mm = screen->availableGeometry() ;
    int width = mm.width();
    int height = mm.height();

    QJsonObject screenObj;
    screenObj.insert("width", width);
    screenObj.insert("height", height);

    qDebug() << Q_FUNC_INFO << "width:" << width << ", height: " << height << endl;
    signalManager()->success(callbackID.toLong(), QVariant(screenObj));
}

void System::getCoreVersion(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(params);
    COsInfo info;
    QString version = info.kernelVersion();
    qDebug() << Q_FUNC_INFO << "version" << version << endl;;
    signalManager()->success(callbackID.toLong(), QVariant(version));
}

void System::getSysVersionID(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(params);
    COsInfo info;
    QString version = info.osVersion();
    qDebug() << Q_FUNC_INFO << "version" << version << endl;;
    signalManager()->success(callbackID.toLong(), QVariant(version));
}
