#include "image.h"
#include "framework/common/errorinfo.h"
#include "helper.h"
#include <QDir>
#include <QDateTime>
#include <QException>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include <QByteArray>

Image::Image()
{
}

void Image::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "  callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    if (actionName == "chooseImage") {
        chooseImage(callbackID, params);
    } else if (actionName == "previewImage") {
        previewImage(callbackID, params);
    } else if (actionName == "saveImageToPhotosAlbum") {
        saveImageToPhotosAlbum(callbackID, params);
    } else if (actionName == "getImageInfo") {
        getImageInfo(callbackID, params);
    }
}

void Image::chooseImage(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params;
    globalCallbackID = callbackID.toLong();

    chooseImageQml = qmlManager.create("qrc:/qml/SChooseImagePage.qml");

    QJsonDocument json(QJsonObject::fromVariantMap(params));
    QString jsonString(json.toJson());

    qmlManager.connectSignal(chooseImageQml, SIGNAL(imageCancel()), this, SLOT(chooseCancel()));
    qmlManager.connectSignal(chooseImageQml, SIGNAL(receiveImageUrls(QString)), this, SLOT(receiveUrls(QString)));
    qmlManager.connectSignal(chooseImageQml, SIGNAL(chooseCameraSuccess(QString)), this, SLOT(cameraSuccess(QString)));
    qmlManager.connectSignal(chooseImageQml, SIGNAL(chooseCameraCancel()), this, SLOT(cameraCancel()));
    qmlManager.connectSignal(chooseImageQml, SIGNAL(chooseAlbumCancel()), this, SLOT(albumCancel()));

    qDebug() << Q_FUNC_INFO << "jsonString" << jsonString;

    qmlManager.call(chooseImageQml, "open(" + jsonString + ")");
}

void Image::chooseCancel()
{
    signalManager()->success(globalCallbackID, "");
    globalCallbackID = 0;
}

void Image::receiveUrls(const QString &filesPath)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(filesPath.toLocal8Bit().data());
    if(jsonDocument.isNull()){
       qDebug()<< "===> please check the string "<< filesPath.toLocal8Bit().data();
    }

    QJsonObject result = jsonDocument.object();

    signalManager()->success(globalCallbackID, QVariant(result));
    qmlManager.destroy(chooseImageQml);
    globalCallbackID = 0;
}

void Image::cameraSuccess(const QString &filePath)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(filePath.toLocal8Bit().data());
    if( jsonDocument.isNull() ){
       qDebug()<< "===> please check the string "<< filePath.toLocal8Bit().data();
    }
    QJsonObject result = jsonDocument.object();

    signalManager()->success(globalCallbackID, result);
    globalCallbackID = 0;
}

void Image::cameraCancel()
{
    signalManager()->success(globalCallbackID, "");
    globalCallbackID = 0;
}

void Image::albumCancel()
{
    signalManager()->success(globalCallbackID, "");
    globalCallbackID = 0;
}

void Image::previewImage(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params;
    globalCallbackID = callbackID.toLong();

    previewQml = qmlManager.open("qrc:/qml/SPreviewImage.qml");

    qmlManager.connectSignal(previewQml, SIGNAL(success()), this, SLOT(previewImageSuccess()));

    QJsonDocument json(QJsonObject::fromVariantMap(params));
    QString jsonString(json.toJson());

    qmlManager.call(previewQml, "showPage(" + jsonString + ")");
}

void Image::previewImageSuccess()
{
    qDebug() << Q_FUNC_INFO;
    signalManager()->success(globalCallbackID, "");
    globalCallbackID = 0;
}

QString Image::getFileName(const QVariantMap &params)
{
    // 文件名
    QString name = params.value("name").toString();
    QString path = params.value("path").toString();
    // 选取的图片路径
    QString filePath = params.value("filePath").toString();
    //设置系统相机路径
    QString _path = Helper::instance()->getInnerStorageRootPath() + "/DCIM";

    QString curPath = path;

    if (path.isEmpty()) {
        curPath = _path;
    }

    QString newFile;

    if (name.isEmpty()) {
        QDir dir(curPath);
        if(!dir.exists()){
            dir.mkpath(curPath);
        }
        QFile file(filePath);
        QFileInfo fileInfo(file);

        //设置文件名
        QDateTime time = QDateTime::currentDateTime();
        //将当前时间转为时间戳
        int timeT = time.toTime_t();
        QStringList filenameArr = fileInfo.fileName().split(".");
        QString filename = filenameArr[0]+"_"+ QString::number(timeT)+"."+filenameArr[1];
        newFile = curPath + "/" + filename;
    } else {
        newFile = curPath + "/" + name;
    }

    return newFile;
}


void Image::saveImageToPhotosAlbum(const QString &callbackID, const QVariantMap &params)
{
    // base64 data
    QString data = params.value("data").toString();

    if (data.isEmpty()) {
        saveImage(callbackID, params);
    } else {
        saveBase64Image(callbackID, params);
    }
}



void Image::saveImage(const QString &callbackID, const QVariantMap &params)
{
    globalCallbackID = callbackID.toLong();
    // 选取的图片路径
    QString filePath = params.value("filePath").toString();

    if (filePath.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "文件路径不能为空" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidParameter, "不合法的参数:filePath不能为空");
        return;
    }

    bool ret = Helper::instance()->isPicture(filePath);
    if (!ret) {
        qDebug() << Q_FUNC_INFO << "不是图片文件" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::IllegalMediaTypeError, "不合法的媒体文件类型:不是图片文件");
        return;
    }

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << Q_FUNC_INFO << "文件不存在：" << filePath << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidURLError, "无效的url:图片不存在");
        return;
    }

    QString newFile = getFileName(params);
    // 是否覆盖
    bool isCover = params.value("isCover").toBool();
    bool isExist = Helper::instance()->exists(newFile);

    if (isCover == false && isExist == true) {
        signalManager()->failed(globalCallbackID, ErrorInfo::SystemError, "系统错误:保存图片失败,文件已存在");
        return;
    }

    try {
        //将照片复制到系统相册
        QFile::copy(filePath, newFile);
    } catch (QException e) {
        qDebug() << Q_FUNC_INFO << "保存图片到系统相册失败" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::SystemError, "系统错误:保存图片到系统相册失败");
        return;
    }
    signalManager()->success(globalCallbackID, newFile);
    globalCallbackID = 0;
}


void Image::saveBase64Image(const QString &callbackID, const QVariantMap &params)
{
    globalCallbackID = callbackID.toLong();
    QString newImgName = getFileName(params);

    // 是否覆盖
    bool isCover = params.value("isCover").toBool();
    bool isExist = Helper::instance()->exists(newImgName);
    if (isCover == false && isExist == true) {
        signalManager()->failed(globalCallbackID, ErrorInfo::SystemError, "系统错误:保存图片失败,文件已存在");
        return;
    }

    // base64 data
    QString data = params.value("data").toString();
    int idx = data.indexOf("base64,");
    data = data.mid(idx + 7, data.length()-1);

    QImage* newImg = base64ToImg(data);

    bool result = newImg->save(newImgName);

    if (result) {
        signalManager()->success(globalCallbackID, newImgName);
    } else {
        signalManager()->failed(globalCallbackID, ErrorInfo::SystemError, "系统错误:保存图片失败");
    }
    delete newImg;
    newImg = NULL;
    globalCallbackID = 0;
}

void Image::getImageInfo(const QString &callbackID, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "params" << params << endl;
    globalCallbackID = callbackID.toLong();

    QString filePath = params.value("path").toString();

    if (filePath.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "文件路径不能为空" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidParameter, "不合法的参数:path不能为空");
        return;
    }

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << Q_FUNC_INFO << "图片不存在：" << filePath << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::InvalidURLError, "无效的url:图片不存在");
        return;
    }

    bool ret = Helper::instance()->isPicture(filePath);
    if (!ret) {
        qDebug() << Q_FUNC_INFO << "不是图片文件" << endl;
        signalManager()->failed(globalCallbackID, ErrorInfo::IllegalMediaTypeError, "不合法的媒体文件类型:不是图片文件");
        return;
    }

    QJsonObject jsonObject;

    QImage imageInfo = QImage(filePath);
    jsonObject.insert("width", imageInfo.width());
    jsonObject.insert("height", imageInfo.height());

    QFileInfo fileInfo(file);
    jsonObject.insert("fileName", fileInfo.fileName());
    jsonObject.insert("size", fileInfo.size());
    jsonObject.insert("created", fileInfo.created().toString("yyyy-MM-dd hh:mm:ss"));

    QStringList filenameArr = fileInfo.fileName().split(".");
    jsonObject.insert("type", filenameArr[filenameArr.length()-1]);

    signalManager()->success(globalCallbackID, QVariant(jsonObject));
    globalCallbackID = 0;
}

//base64 转 QImage
QImage* Image::base64ToImg(const QString & str)
{
    qDebug() << Q_FUNC_INFO << str;
    QImage* img = new QImage;
    QByteArray arr_base64 = str.toLatin1();
    img->loadFromData(QByteArray::fromBase64(arr_base64));
    return img;
}
