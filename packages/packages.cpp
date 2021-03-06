#include "packages.h"
#include "cgui_application.h"
#include <QDebug>
#include "framework/common/errorinfo.h"

using namespace NativeSdk;

Packages::Packages()
{
}

void Packages::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "openUrl") {
        QString scheme = params.value("scheme").toString();
        QString path = params.value("path").toString();
        QVariantMap pathParams = params.value("params").toMap();

        openUrl(callbackID, scheme, path, pathParams);

    } else if (actionName == "openByUrl") {
        QString url = params.value("url").toString();
        qDebug() << "url:" << url << endl;

        openByUrl(url);

    } else if (actionName == "openDocument") {
        QString sopId = params.value("sopid").toString();
        QString uiappId = params.value("uiappid").toString();
        QString action = params.value("action").toString();
        QString path = "index.html";
        QVariantMap pathParams = params.value("params").toMap();

        openDocument(callbackID, sopId, uiappId, action, path, pathParams);

    } else if (actionName == "openByDocument") {
        QString action = params.value("action").toString();
        QString mimetype = params.value("mimetype").toString();
        QString filePath = params.value("filePath").toString();
        
        openByDocument(action, mimetype, filePath);
    }
}

void Packages::openUrl(const QString &callbackID, const QString &scheme, const QString &path, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "scheme:" << scheme << path << params;
    using namespace SYBEROS;

    if (scheme.isEmpty()) {
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "scheme参数不能为空");
        return;
    }

    QStringList schemeList = scheme.split(":");
    if (schemeList.size() != 2) {
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "scheme参数不合法");
        return;
    }

    if (schemeList.value(1).indexOf("openPage") >= 0) {
        openPage(callbackID, scheme, path, params);
    } else {
        qApp->openUrl(scheme);
        signalManager()->success(callbackID.toLong(), "success");
    }
}

void Packages::openPage(const QString &callbackID, const QString &scheme, const QString &path, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "scheme:" << scheme << path << params;
    using namespace SYBEROS;

    //遍历params拼接成key=value&key=value格式
    QString paramStr = "";
    QMap<QString, QVariant>::const_iterator it = params.constBegin();
    int i = 0;
    while(it != params.constEnd()) {
        if (i > 0) {
            paramStr.append("&");
        }
        paramStr.append(it.key());
        paramStr.append("=");
        paramStr.append(it.value().toString());
        it++;
        i++;
    }

    //如果路径包含多个?，提示路径错误
    QStringList list = path.split("?");

    QString openPath;
    if (list.size() == 1) {
        openPath = path + "?" + paramStr;
    } else {
        openPath = path + "&" + paramStr;
    }
    openPath = scheme + "/" + openPath;
    //唤起APP
    qDebug() << "唤起APP的路径: " << openPath << endl;
    qApp->openUrl(openPath);
    signalManager()->success(callbackID.toLong(), "success");
}

void Packages::openByUrl(const QString &url) {
    qDebug() << Q_FUNC_INFO << "  url:" << url;

    //url格式为: scheme://openPage/index.html?key=value&key=value
    QStringList list = url.split("://");
    if (list.size() != 2) {
        return;
    }

    QString methodAndPathStr = list.value(1);
    QStringList methodAndPath = methodAndPathStr.split("/");
    if (methodAndPath.size() < 1) {
        return;
    }

    //判断方法是否为openPage
    QString method = methodAndPath.value(0);
    if (method.toLower() != "openpage") {
        return;
    }

    QString path = "";
    for (int i = 1; i < methodAndPath.size(); i++) {
        if (i > 1) {
            path.append("/");
        }
        path.append(methodAndPath.value(i));
    }

    //将参数转换为QJsonObject
    QJsonObject query;
    QStringList pathList = path.split("?");
    if (pathList.size() == 2) {
        QStringList list = pathList.value(1).split("&");
        for (int i = 0; i < list.size(); i++) {
            QString item = list.value(i);
            QStringList itemList = item.split("=");
            if (itemList.size() != 2) {
                continue;
            }
            query.insert(itemList.value(0), itemList.value(1));
        }
    }

    qDebug() << "openByUrl path: " << path << endl;
    qDebug() << "openByUrl query: " << query << endl;

    QVariantMap result;
    if (path.isEmpty()) {
        path = "index.html";
    }
    result.insert("path", path);
    result.insert("params", query);

    signalManager()->subscribe("Packages", result);
}

void Packages::openDocument(const QString &callbackID, const QString &sopId, const QString &uiappId,
                  const QString &action, const QString &path, const QVariantMap &params){

    qDebug() << Q_FUNC_INFO << "  params:" << callbackID << sopId << uiappId << action << path << params;
    using namespace SYBEROS;

    if (sopId.isEmpty()) {
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "sopid is empty");
        return;
    }

    if (uiappId.isEmpty()) {
        signalManager()->failed(callbackID.toLong(), ErrorInfo::InvalidParameter, "uiappid is empty");
        return;
    }

    QString mimeType = params.value("mimeType").toString();
    if (mimeType.isEmpty()) {
        mimeType = "*";
    }
    QString filePath = params.value("filePath").toString();

    if (action != "openPage") {
        qApp->runDocument(sopId, uiappId, action, mimeType, filePath);
        signalManager()->success(callbackID.toLong(), "success");
        return;
    }

    QVariantMap pathParam;
    QVariantMap::const_iterator it = params.constBegin();
    while(it != params.constEnd())
    {
        if(it.key() == "filePath" || it.key() == "mimeType"){
            it++;
            continue;
        }
        pathParam.insert(it.key(), it.value().toString());
        it++;
    }
    pathParam.insert("action", action);

    QString newPath = convertParamToUrl(path, pathParam);
    qDebug() << "openDocument request: " << newPath << endl;

    qApp->runDocument(sopId, uiappId, newPath, mimeType, filePath);
    signalManager()->success(callbackID.toLong(), "success");
}

void Packages::openByDocument(const QString &action, const QString &mimetype, const QString &filePath){
    qDebug() << Q_FUNC_INFO << "  params:" << action << mimetype << filePath;

    QString path = action;
    if(path.isEmpty()){
        path = "index.html";
    }

    QVariantMap params;
    params = parseUrlToParam(path, params);
    params.insert("mimeType", mimetype);
    params.insert("filePath", filePath);

    QJsonObject query;
    QVariantMap::Iterator it = params.begin();
    while(it != params.end())
    {
        query.insert(it.key(), it.value().toString());
        it++;
    }

    path = convertParamToUrl(path, params);
    QVariantMap result;
    result.insert("path", path);
    result.insert("params", query);

    qDebug() << "final subscribe result: " << result << endl;

    signalManager()->subscribe("Packages", result);
}

QString Packages::convertParamToUrl(const QString &url, const QVariantMap &paramMap){

    QMap<QString, QVariant>::const_iterator it = paramMap.constBegin();

    QString paramStr;
    int i = 0;
    while(it != paramMap.constEnd()) {
        if (i > 0) {
            paramStr.append("&");
        }
        paramStr.append(it.key());
        paramStr.append("=");
        paramStr.append(it.value().toString());
        it++;
        i++;
    }

    QString newUrl;
    if(url.indexOf("?") > -1){
        newUrl = url + "&" + paramStr;
    }else{
        newUrl = url + "?" + paramStr;
    }
    return newUrl;
}

QVariantMap Packages::parseUrlToParam(const QString &url, QVariantMap paramMap){
    if (url.isEmpty()) {
        return paramMap;
    }

    QStringList list = url.split("?");
    if (list.size() < 2) {
        return paramMap;
    }

    QStringList paramList = list.value(1).split("&");

    for (int i = 0; i < paramList.size(); i++) {
        QString item = paramList.value(i);
        QStringList itemList = item.split("=");
        if (itemList.size() != 2) {
            continue;
        }
        paramMap.insert(itemList.value(0), itemList.value(1));
    }
    return paramMap;
}
