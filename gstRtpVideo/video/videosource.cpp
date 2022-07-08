#include "videosource.h"

VideoSource::VideoSource(QObject *parent) : QObject(parent)
{
}

VideoSource::~VideoSource()
{
    if(m_surface){
        m_surface->stop();
        m_surface->deleteLater();
        m_surface = nullptr;
    }
}

void VideoSource::setVideoSurface(QAbstractVideoSurface *surface)
{
    // if (m_surface && m_surface != surface && m_surface->isActive()) {
    //     m_surface->stop();
    // }

    m_surface = surface;

    QSize qsize(m_width, m_height);
    QVideoSurfaceFormat format(qsize, m_format);
    if (m_surface)
        m_surface->start(format);
}

void VideoSource::newVideoFrame(const QVideoFrame& frame)
{
    if (m_surface){
        m_surface->present(frame);
    }
}

void VideoSource::setFormat(int width, int height, QVideoFrame::PixelFormat format)
{
    m_width = width;
    m_height = height;
    m_format = format;
}
