#pragma once

#include "constants.h"
#include "models/qqmlvariantlistmodel.h"

#include <QObject>
#include <QTimer>
#include <QVariantList>


class FFTVisualizationData : public QObject
{
Q_OBJECT;

public:

    FFTVisualizationData(Visualizer::Common::fft_result_queue_t& queue,
                         QObject* parent = nullptr);

    ~FFTVisualizationData() = default;

    QQmlVariantListModel* getModel()
    {
        return &m_model;
    }


private:

    Visualizer::Common::fft_result_queue_t& m_queue;
    QTimer m_timer;

    QQmlVariantListModel m_model;

};
