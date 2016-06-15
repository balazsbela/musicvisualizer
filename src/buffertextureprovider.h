#pragma once

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <QSharedPointer>

class BufferTextureProvider : public QQuickImageProvider
{

public:

    explicit BufferTextureProvider();
    virtual ~BufferTextureProvider() = default;

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setPixel(const QPoint &position, uint color);
    void setPixel(const QPoint &position, const QColor& color) ;

private:

    QImage m_image;

};
