#include "path.h"
#include <QDebug>
#include <QJsonObject>
#include "helper.h"
#include "framework/common/errorinfo.h"

using namespace NativeSdk;

Path::Path()
{
}


void Path::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    Q_UNUSED(params);
    
    if (actionName == "extern") {
        getExternStorageRootPath(callbackID);
    } else if (actionName == "inner") {
        getInnerStorageRootPath(callbackID);
    } else if (actionName == "dataRoot") {
        getDataRootPath(callbackID);
    }
}

void Path::getExternStorageRootPath(const QString &callbackID)
{
    Helper *helper = Helper::instance();
    QString ext = helper->getExternStorageRootPath();
    QJsonObject jsObj;
    jsObj.insert("result", ext);
    signalManager()->success(callbackID.toLong(), jsObj);
}

void Path::getInnerStorageRootPath(const QString &callbackID)
{
    Helper *helper = Helper::instance();
    QString inner = helper->getInnerStorageRootPath();
    QJsonObject jsObj;
    jsObj.insert("result", inner);
    signalManager()->success(callbackID.toLong(), jsObj);
}

void Path::getDataRootPath(const QString &callbackID)
{
    Helper *helper = Helper::instance();
    QString dataRoot = helper->getDataRootPath();
    QJsonObject jsObj;
    jsObj.insert("result", dataRoot);
    signalManager()->success(callbackID.toLong(), jsObj);
}
