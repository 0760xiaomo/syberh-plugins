#ifndef GSTRTPVIDEO_H
#define GSTRTPVIDEO_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "gstRtpVideo_global.h"


class GstRtpVideoPrivate;

class GSTRTPVIDEOSHARED_EXPORT GstRtpVideo: public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    GstRtpVideo();
    virtual ~GstRtpVideo();

    void invokeInitialize();
    void invoke(QString callbackID, QString actionName, QVariantMap params);

private:
    GstRtpVideoPrivate *d;
};

#endif // GSTRTPVIDEO_H
