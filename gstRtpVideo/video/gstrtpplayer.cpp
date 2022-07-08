#include "gstrtpplayer.h"

#include <QDebug>
#include <private/qmemoryvideobuffer_p.h>
#include "gst/app/gstappsink.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

GstRtpPlayer::GstRtpPlayer(QObject *parent) : QObject(parent)
{
}

GstRtpPlayer::~GstRtpPlayer()
{
    if(mPipeline != nullptr){
        stop();
        gst_object_unref (GST_OBJECT (mPipeline));
    }
}

void GstRtpPlayer::initPipeline(QString address, int port)
{
    gst_init(NULL, NULL);

    mPipeline = gst_pipeline_new("GstRtpPlayer");
    mUdpSrc = gst_element_factory_make("udpsrc", "udpsrc");
    mUdpSrcCF = gst_element_factory_make("capsfilter", "udpsrccf");
    mRtpH264Depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    mRtpH264DepayCF = gst_element_factory_make("capsfilter", "rtph264depaycf");
    mDecodebin = gst_element_factory_make("decodebin2", "decodebin");
    mDecodeBinCF = gst_element_factory_make("capsfilter", "decodebincf");
    mAppSink = gst_element_factory_make("appsink", "appsink");

    if(!mPipeline || !mUdpSrc || !mUdpSrcCF || !mRtpH264Depay || !mRtpH264DepayCF || !mDecodebin || !mDecodeBinCF || !mAppSink){
        g_printerr("Some elements could not be created");
        return;
    }

    GstCaps *udpSrcCaps = gst_caps_new_simple("application/x-rtp",
                                              "media", G_TYPE_STRING, "video",
                                              "payload", G_TYPE_INT, 110,
                                              "clock-rate", G_TYPE_INT, 90000,
                                              "encoding-name", G_TYPE_STRING, "H264",
                                              NULL);

    GstCaps *mtpH264DepayCaps = gst_caps_new_simple("video/x-h264",
                                                    "width", G_TYPE_INT, 720,
                                                    "height", G_TYPE_INT, 576,
                                                    "framerate", GST_TYPE_FRACTION, 25, 1,
                                                    NULL);

    GstCaps *decodebinCaps = gst_caps_new_simple("video/x-raw-yuv",
                                                 NULL);

    g_object_set(G_OBJECT(mUdpSrc), "multicast_group", address.toStdString().c_str(), NULL);
    g_object_set(G_OBJECT(mUdpSrc), "port", port, NULL);
    g_object_set(G_OBJECT(mUdpSrcCF), "caps", udpSrcCaps, NULL);
    g_object_set(G_OBJECT(mRtpH264DepayCF), "caps", mtpH264DepayCaps, NULL);
    g_object_set(G_OBJECT(mDecodeBinCF), "caps", decodebinCaps, NULL);
    g_object_set(G_OBJECT(mAppSink), "emit-signals", TRUE, NULL);

    /* we add a message handler */
    mBus = gst_pipeline_get_bus (GST_PIPELINE (mPipeline));
    gst_object_unref (mBus);
    gst_bus_add_signal_watch (mBus);

    gst_bin_add_many (GST_BIN (mPipeline), mUdpSrc, mUdpSrcCF, mRtpH264Depay, mRtpH264DepayCF, mDecodebin, mDecodeBinCF, mAppSink, NULL);
    g_signal_connect (mAppSink, "new-buffer", G_CALLBACK (onNewBuffer), this);
    g_signal_connect (mBus, "message", G_CALLBACK (onMessage), this);
    g_signal_connect (mDecodebin, "pad-added", G_CALLBACK (onPadAdded), this);

    if(!gst_element_link(mUdpSrc, mUdpSrcCF)){
       g_warning("failed to link udpsrc and capsfilter");
       return;
    }

    if(!gst_element_link(mUdpSrcCF, mRtpH264Depay)){
       g_warning("failed to link capsfilter and rtph264depay");
       return;
    }

    if(!gst_element_link(mRtpH264Depay, mRtpH264DepayCF)){
       g_warning("failed to link rtph264depay and decodebin");
       return;
    }

    if(!gst_element_link(mRtpH264DepayCF, mDecodebin)){
       g_warning("failed to link capsfilter and decodebin");
       return;
    }

    if(!gst_element_link(mDecodeBinCF, mAppSink)){
       g_warning("failed to link capsfilter and appsink");
       return;
    }
}

void GstRtpPlayer::start()
{
    gst_element_set_state (mPipeline, GST_STATE_PLAYING);
}

void GstRtpPlayer::stop()
{
    gst_element_set_state (mPipeline, GST_STATE_NULL);
}

//static
void GstRtpPlayer::onPadAdded (GstElement *element, GstPad *pad, gpointer user_data)
{
    Q_UNUSED(pad)
    qDebug() << __func__ << "-------------------------->>>>>>>>>>>>>>>>>>>>>";
    GstRtpPlayer *player = (GstRtpPlayer*)user_data;
    if(!gst_element_link(element, player->mDecodeBinCF)){
       g_warning("failed to link decodebin2 and decodebincf");
       return;
    }
}

void GstRtpPlayer::saveVideoData(const QByteArray &videoData)
{
    int dataFd = open("/home/user/videodata.dat", O_WRONLY|O_APPEND|O_CREAT);
    if (dataFd > 0) {
        write(dataFd, videoData.data(), videoData.length());
        close(dataFd);
    }
}


void GstRtpPlayer::onNewBuffer(GstElement *sink, gpointer user_data)
{
    QByteArray videoData;

    GstRtpPlayer *player = (GstRtpPlayer*)user_data;
    GstBuffer *buffer = gst_app_sink_pull_buffer ((GstAppSink*)sink);

    int width = 720, height = 576;
    int bufferSize = GST_BUFFER_SIZE(buffer);
    QSize qsize(width, height);

    qDebug() << __FILE__ << ":" << __LINE__;
    videoData.replace(0, bufferSize, (const char*)GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));

    gst_buffer_unref(buffer);

    QMemoryVideoBuffer *videoBuffer = new QMemoryVideoBuffer(videoData, width);
    QVideoFrame frame((QAbstractVideoBuffer*)videoBuffer, qsize, QVideoFrame::Format_NV12);

    emit player->newVideoFrame(frame);
//    saveVideoData(videoData);
}
//static
void GstRtpPlayer::onMessage (GstBus *bus, GstMessage *msg, gpointer user_data)
{
    Q_UNUSED(bus)
    qDebug() << "cb_message";
    GstRtpPlayer *gstRtpPlayer = (GstRtpPlayer*)user_data;
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;

        gst_message_parse_error (msg, &err, &debug);
        g_print ("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);

        gst_element_set_state (gstRtpPlayer->mPipeline, GST_STATE_READY);
        break;
    }
    case GST_MESSAGE_EOS:
        qDebug()<<"EOS !!!!!!";
        /* end-of-stream */
        gst_element_set_state (gstRtpPlayer->mPipeline, GST_STATE_NULL);
        break;
    case GST_MESSAGE_BUFFERING: {
        gint percent = 0;

        gst_message_parse_buffering (msg, &percent);
        g_print ("Buffering (%3d%%)\r", percent);
        break;
    }
    case GST_MESSAGE_CLOCK_LOST:
        /* Get a new clock */
        gst_element_set_state (gstRtpPlayer->mPipeline, GST_STATE_PAUSED);
        gst_element_set_state (gstRtpPlayer->mPipeline, GST_STATE_PLAYING);
        break;
    case GST_MESSAGE_STATE_CHANGED:
        GstState stateOld;
        GstState stateNew;

        gst_message_parse_state_changed(msg, &stateOld, &stateNew, NULL);
        qDebug() << "STATE CHANGE: "
                 << "NAME: " << QString(GST_OBJECT_NAME (msg->src))
                 << " Old State: "  << QString(gst_element_state_get_name (stateOld))
                 << " New State: "  << QString(gst_element_state_get_name (stateNew));
        break;
    default:
        /* Unhandled message */
        break;
    }

}



