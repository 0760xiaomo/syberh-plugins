#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QObject>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QDebug>

class VideoSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractVideoSurface *videoSurface READ videoSurface WRITE setVideoSurface)

public:
    VideoSource(QObject *parent = 0);
    virtual ~VideoSource();

    QAbstractVideoSurface* videoSurface() const { return m_surface; }

    void setVideoSurface(QAbstractVideoSurface *surface);

    Q_INVOKABLE void setFormat(int width, int height, QVideoFrame::PixelFormat);

public slots:
    void newVideoFrame(const QVideoFrame& frame);

private:
    QAbstractVideoSurface *m_surface;
    QVideoFrame::PixelFormat m_format;
    int m_width;
    int m_height;
};

#endif // VIDEOSOURCE_H
