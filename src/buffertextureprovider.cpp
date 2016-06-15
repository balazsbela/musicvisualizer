#include "buffertextureprovider.h"
#include "constants.h"

#include <QDebug>


BufferTextureProvider::BufferTextureProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image),
      m_image(QSize(Visualizer::Common::nrSamples, 1), QImage::Format_ARGB32)
{

}


BufferTextureProvider::~BufferTextureProvider()
{
    qDebug() << "Deleted!";
}


QImage BufferTextureProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    *size = m_image.size();

//    qDebug() << id << *size << " " << requestedSize.width() << requestedSize.height();

    return m_image;
}


void BufferTextureProvider::setPixel(const QPoint &position, uint color)
{
    m_image.setPixel(position, color);
}


void BufferTextureProvider::setPixel(const QPoint &position, const QColor& color)
{
    m_image.setPixelColor(position, color);
}

