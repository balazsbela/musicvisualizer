#pragma once

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <QSharedPointer>

class BufferTextureProvider : public QQuickImageProvider
{

public:

    BufferTextureProvider();
    ~BufferTextureProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setPixel(const QPoint &position, uint color);
    void setPixel(const QPoint &position, const QColor& color) ;

    void setImage(QSharedPointer<QImage> image)
    {
        m_image = image;
    }

private:

    QSharedPointer<QImage> m_image;

};
