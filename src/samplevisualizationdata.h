#pragma once

#include "constants.h"
#include "models/qqmlvariantlistmodel.h"

#include <QObject>
#include <QTimer>
#include <QVariantList>


class SampleVisualizationData : public QObject
{
Q_OBJECT;

public:

    SampleVisualizationData(Visualizer::Common::sample_queue_t& queue,
                            QObject* parent = nullptr);

    ~SampleVisualizationData() = default;

    QQmlVariantListModel* getModel()
    {
        return &m_model;
    }


private:

    Visualizer::Common::sample_queue_t& m_queue;
    QTimer m_timer;

    QQmlVariantListModel m_model;

};
