#include "battery.h"
#include <QObject>
#include <QDebug>
#include <cosinfo.h>
#include <csystemdeviceinfo.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <cbatterymanager.h>


Battery::Battery()
{
}


void Battery::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    //如果参数不使用此方法可以防止运行异常
    Q_UNUSED(actionName);
    getStatus(callbackID, params);
}

void Battery::getStatus(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(params);

    CBatteryManager battery;
    int capacity = battery.capacity();
    int level = battery.level();
    bool isValid = battery.isValid();
    bool isCharging = battery.isCharging();

    QJsonObject json;
    json.insert("isCharging", isCharging);
    json.insert("level", level);
    json.insert("capacity", capacity);
    json.insert("isValid", isValid);

    qDebug() << "getStatus: " << json << endl;

    signalManager()->success(callbackID.toLong(), QVariant(json));
}
