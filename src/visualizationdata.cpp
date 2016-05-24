#include "visualizationdata.h"


VisualizationData::VisualizationData(Visualizer::Constants::fft_result_queue_t& queue,
                                     QObject* parent)
    : QObject(parent)
    , m_queue(queue)
{
    m_timer.setInterval(0);
    m_timer.setSingleShot(false);

    QObject::connect(&m_timer, &QTimer::timeout, [&]()
    {
        Visualizer::Constants::fft_result result;
        while (m_queue.pop(result))
        {
            QVariantList list;

            for (int i = 0; i < Visualizer::Constants::fftResultSize; ++i)
            {
                list.append(qreal(result[i]));
            }

            emit dataAvailable(std::move(list));
        }
    });

    m_timer.start();
}
