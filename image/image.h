#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "image_global.h"
#include "qmlmanager.h"

using namespace NativeSdk;

class IMAGESHARED_EXPORT Image: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    Q_INVOKABLE Image();

    void invoke(const QString &callbackID, const QString &action, const QVariantMap &params);

    void chooseImage(const QString &callbackID, const QVariantMap &params);
    void previewImage(const QString &callbackID, const QVariantMap &params);
    //base64 转 QImage
    QImage* base64ToImg(const QString &str);

    /**
     * @brief getFileName 拼接新的图片名
     * @return 成功则返回图片的绝对路径
     */
    QString getFileName(const QVariantMap &params);

private:
    QmlManager qmlManager;
    QmlObject *chooseImageQml;
    QmlObject *previewQml;
    long globalCallbackID;

    /**
     * @brief saveImageToPhotosAlbum 保存图片到相册
     * @param callBackID 唤起应用的任务Id
     */
    void saveImageToPhotosAlbum(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief saveImage 保存图片文件
     * @param callBackID 唤起应用的任务Id
     * @param filePath 文件路径
     * @return 成功则返回图片路径。
     *      失败则发送失败信号。
     */
    void saveImage(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief saveBase64Image 保存base64格式的图片
     * @param callBackID 唤起应用的任务Id
     * @return 成功则返回图片路径。
     *      失败则发送失败信号。
     */
    void saveBase64Image(const QString &callbackID, const QVariantMap &params);

    /**
     * @brief getImageInfo 获取图片的详细
     * @param callBackID 任务Id
     * @return 成功则发送成功信号。
     *      失败则发送失败信号。
     */
    void getImageInfo(const QString &callbackID, const QVariantMap &params);

public slots:
    void chooseCancel();
    void receiveUrls(const QString &filesPath);
    void cameraSuccess(const QString &filePath);
    void cameraCancel();
    void albumCancel();
    void previewImageSuccess();
};

#endif // IMAGE_H
