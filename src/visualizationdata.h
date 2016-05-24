#pragma once

#include "constants.h"
#include "models/qqmlvariantlistmodel.h"

#include <QObject>
#include <QTimer>
#include <QVariantList>


class VisualizationData : public QObject
{
Q_OBJECT;

public:

    VisualizationData(Visualizer::Constants::fft_result_queue_t& queue,
                      QQmlVariantListModel& model,
                      QObject* parent = nullptr);

    ~VisualizationData() = default;

private:

    Visualizer::Constants::fft_result_queue_t& m_queue;
    QTimer m_timer;

    QQmlVariantListModel& m_model;

};
