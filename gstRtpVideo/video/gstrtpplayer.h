#ifndef GSTRTPPLAYER_H
#define GSTRTPPLAYER_H

#include <QObject>
#include <QVideoFrame>
#include <gst/gst.h>

class GstRtpPlayer : public QObject
{
    Q_OBJECT
public:
    explicit GstRtpPlayer(QObject *parent = 0);
    virtual ~GstRtpPlayer();

    typedef struct{
        void *buffer;
        size_t size;
    }RtpBuffer;

    Q_INVOKABLE void initPipeline(QString address, int port);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    static void onNewBuffer(GstElement *element, gpointer user_data);
    static void onMessage (GstBus *bus, GstMessage *msg, gpointer user_data);
    static void onPadAdded (GstElement *element, GstPad *pad, gpointer user_data);
    static void saveVideoData(const QByteArray &videoData);
signals:
    void newVideoFrame(const QVideoFrame &frame);

private:
    GstElement *mPipeline;
    GstElement *mUdpSrc;
    GstElement *mUdpSrcCF;
    GstElement *mRtpH264Depay;
    GstElement *mRtpH264DepayCF;
    GstElement *mDecodebin;
    GstElement *mDecodeBinCF;
    GstElement *mAppSink;
    GstBus *mBus;
};

#endif // GSTRTPPLAYER_H
