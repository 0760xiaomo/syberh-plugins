#include "networkinfo.h"
#include <QObject>
#include <QDebug>
#include <QJsonObject>
#include <cnetworkmanager.h>

NetworkInfo::NetworkInfo()
{
}


void NetworkInfo::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "info") {
        info(callbackID, params);
    }
}

void NetworkInfo::info(const QString &callbackID, const QVariantMap &params){
    Q_UNUSED(params);

    CNetworkManager network;
    bool isNetworkAvailable = network.isNetworkAvailable();
    bool isWifiConnected = network.isWifiConnected();
//    int networkType = CNetworkManager::NetworkType;
    int wifiSignalStrength = network.wifiSignalStrength();
    int networkType = network.currentNetworkType();

    QJsonObject json;
    json.insert("networkType", networkType);
    json.insert("isNetworkAvailable", isNetworkAvailable);
    json.insert("isWifiConnected", isWifiConnected);
    json.insert("wifiSignalStrength", wifiSignalStrength);

    qDebug() << Q_FUNC_INFO << "json:" << json << endl;
    signalManager()->success(callbackID.toLong(),  QVariant(json));
}

