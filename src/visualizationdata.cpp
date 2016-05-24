#include "visualizationdata.h"


VisualizationData::VisualizationData(Visualizer::Constants::fft_result_queue_t& queue,
                                     QQmlVariantListModel& model,
                                     QObject* parent)
    : QObject(parent)
    , m_queue(queue)
    , m_model(model)
{
    m_timer.setInterval(0);
    m_timer.setSingleShot(false);

    QObject::connect(&m_timer, &QTimer::timeout, [&]()
    {
        Visualizer::Constants::fft_result result;
        while (m_queue.pop(result))
        {
            for (int i = 0; i < Visualizer::Constants::fftResultSize; ++i)
            {
                QVariantMap element;
                element["val"] = qreal(result[i]);

                if (m_model.count() < i)
                {
                    m_model.append(element);
                }
                else
                {
                    m_model.replace(i, element);
                }
            }
        }
    });

    m_timer.start();
}
