#ifndef GSTRTPVIDEO_P_H
#define GSTRTPVIDEO_P_H

#include "qmlmanager.h"
#include "gstRtpVideo.h"

#include <QObject>
#include <QMap>

using namespace NativeSdk;

class GstRtpVideoPrivate : public QObject 
{
    Q_OBJECT
public:
    GstRtpVideoPrivate(QObject *parent = nullptr);
    virtual ~GstRtpVideoPrivate();

    /** 创建视频组件 */
    void create(const QString &callbackID, const QVariantMap &params);
    /** 删除指定的视频组件 */
    void remove(const QString &callbackID, const QVariantMap &params);
    /** 删除所有视频组件 */
    void removeAll(const QString &callbackID);
    /** 开始播放 */
    void start(const QString &callbackID, const QVariantMap &params);
    /** 停止播放 */
    void stop(const QString &callbackID, const QVariantMap &params);
    /** 显示视频组件 */
    void show(const QString &callbackID, const QVariantMap &params);
    /** 隐藏视频组件 */
    void hide(const QString &callbackID, const QVariantMap &params);

public slots:
    void videoEvent(QString videoId, QString eventType, QVariant eventData);

    void cpagePoped(QString videoId);

private:
    GstRtpVideo *p = nullptr;

    QmlManager m_qmlManager;
    QMap<QString, QmlObject*> m_qmlObjects;

    void clearQmlObjects();
    QString destoryQmlObject(const QString &videoId);
};
#endif // GSTRTPVIDEO_P_H