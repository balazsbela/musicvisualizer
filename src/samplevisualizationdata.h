#pragma once

#include "constants.h"
#include "models/qqmlvariantlistmodel.h"
#include "buffertextureprovider.h"

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QImage>


class SampleVisualizationData : public QObject
{
Q_OBJECT;

public:

    SampleVisualizationData(Visualizer::Common::sample_queue_t& queue,
                            BufferTextureProvider&  bufferTextureProvider,
                            QObject* parent = nullptr);

    ~SampleVisualizationData() = default;

    Q_INVOKABLE QQmlVariantListModel* getModel()
    {
        return &m_model;
    }


private:

    Visualizer::Common::sample_queue_t& m_queue;
    QTimer m_timer;

    QQmlVariantListModel        m_model;
    BufferTextureProvider&      m_bufferTextureProvider;

};
