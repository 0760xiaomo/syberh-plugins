#include <QDebug>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <csystemnetworkmanager.h>

#include "cellular.h"


Cellular::Cellular()
{
}


void Cellular::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName=="networkEnabled"){
        networkEnabled(callbackID, params);
    }else if (actionName=="networkDisabled"){
        networkDisabled(callbackID, params);
    }
}

void Cellular::networkDisabled(const QString &callbackID, const QVariantMap &params){

    qDebug() << "NetworkSetCellularEnabled callbackID is" << callbackID << "params is" << params << endl;

    bool flag = false;

    CSystemNetworkManager networkManager;
    //true 表示可用， false 表示禁用。
    bool isDisabled = networkManager.setCellularEnabled(flag);

    QJsonObject json;
    json.insert("isDisabled", isDisabled);

    qDebug() << "isDisabled: " << isDisabled << endl;

    signalManager()->success(callbackID.toLong(), json);
}


void Cellular::networkEnabled(const QString &callbackID, const QVariantMap &params){

    qDebug() << "NetworkSetCellularEnabled callbackID is" << callbackID << "params is" << params << endl;

    bool flag = true;

    CSystemNetworkManager networkManager;
    //true 表示可用， false 表示禁用。
    bool isEnabled = networkManager.setCellularEnabled(flag);

    QJsonObject json;
    json.insert("isEnabled", isEnabled);

    qDebug() << "isEnabled: " << isEnabled << endl;

    signalManager()->success(callbackID.toLong(), json);
}

