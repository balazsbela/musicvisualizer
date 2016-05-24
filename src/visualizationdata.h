#pragma once

#include "constants.h"

#include <QObject>
#include <QTimer>
#include <QVariantList>


class VisualizationData : public QObject
{
Q_OBJECT;

public:

    VisualizationData(Visualizer::Constants::fft_result_queue_t& queue,
                      QObject* parent = nullptr);

    ~VisualizationData() = default;

signals:

    void dataAvailable(const QVariantList& resultList);


private:

    Visualizer::Constants::fft_result_queue_t& m_queue;
    QTimer m_timer;

};
