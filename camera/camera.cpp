#include <QDir>
#include <QDateTime>
#include <QException>
#include "camera.h"
#include "framework/common/errorinfo.h"
#include "helper.h"
#include <SyberosGuiCache>
#include <QKeyEvent>

Camera::Camera()
{
}

void Camera::extensionsInitialized()
{
    qDebug() << Q_FUNC_INFO;
    // error signal
    connect(&qmlManager, &QmlManager::error, this, &Camera::error);
}

void Camera::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "takePhoto") {
        takePhoto(callbackID, params);
    } else if (actionName == "changeCameraImagePath") {
        changeCameraImagePath(callbackID, params);
    }
}

void Camera::takePhoto(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params;
    globalCallbackID = callbackID.toLong();

    QString enableCut = params.value("enableCut").toString();

    QQuickItem* currentItem = qmlManager.currentItem();
    qDebug() << Q_FUNC_INFO << "currentItem***************" << currentItem;

    QVariant page = qmlManager.call(currentItem, "var webviewPage = pageStack.currentPage;console.log(webviewPage);var cameraComponent = pageStack.push('qrc:/qml/SCamera.qml', {'pageItem': webviewPage, 'enableCut':"+ enableCut +"});cameraComponent");

    qDebug() << Q_FUNC_INFO << "page" << page;

    cameraQml = page.value<QQuickItem*>();

    connect(cameraQml, SIGNAL(imageConfirmed(QString, bool)), this, SLOT(imageConfirmed(QString)));
    connect(cameraQml, SIGNAL(imageCancele()), this, SLOT(imageCancele()));
}

void Camera::imageConfirmed(const QString &filePath)
{
    qDebug() << Q_FUNC_INFO << "filePath" << filePath<< "****************************";

    QJsonObject jsonObject;
    jsonObject.insert("path", filePath);

    signalManager()->success(globalCallbackID, QVariant(jsonObject));

    globalCallbackID = 0;

    qmlManager.call(cameraQml, "cameraBack()");
}

void Camera::error(const QString &errorMsg)
{
    qDebug() << Q_FUNC_INFO << "errorMsg::" << errorMsg;
    globalCallbackID = 0;
}


void Camera::imageCancele()
{
    qDebug() << Q_FUNC_INFO << "****************************";

    signalManager()->success(globalCallbackID, "");
    globalCallbackID = 0;
}


void Camera::changeCameraImagePath(const QString &callbackID, const QVariantMap &params){
    qDebug() << Q_FUNC_INFO << "changeCameraImagePath" << params << endl;
    globalCallbackID = callbackID.toLong();

    QString filePath = params.value("path").toString();

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << Q_FUNC_INFO << "????????????????????????" << filePath << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidURLError, "?????????url:?????????????????????");
        return;
    }

    //????????????????????????
    QString path = Helper::instance()->getInnerStorageRootPath() + "/DCIM";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    QFileInfo fileInfo(file);

    //???????????????
    QDateTime time = QDateTime::currentDateTime();   //??????????????????
    int timeT = time.toTime_t();                     //??????????????????????????????
    QStringList filenameArr = fileInfo.fileName().split(".");
    QString filename = filenameArr[0]+"_"+ QString::number(timeT)+"."+filenameArr[1];
    QString newFile = path + "/" + filename;

    try  {
        //?????????????????????????????????????????????????????????
        QFile::copy(filePath, newFile);
        file.remove();
    } catch (QException e) {
        qDebug() << Q_FUNC_INFO << "????????????????????????????????????" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::SystemError, "????????????:????????????????????????????????????");
        return;
    }

    QJsonObject jsonObject;
    jsonObject.insert("path", newFile);
    QJsonValue::fromVariant(jsonObject);
    signalManager()->success(globalCallbackID, QVariant(jsonObject));
    globalCallbackID = 0;
}

