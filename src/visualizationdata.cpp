#include "visualizationdata.h"
#include "constants.h"

VisualizationData::VisualizationData(Visualizer::Common::fft_result_queue_t& queue,
                                     QQmlVariantListModel& model,
                                     QObject* parent)
    : QObject(parent)
    , m_queue(queue)
    , m_model(model)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(Visualizer::Common::fftTimerInterval);
    m_timer.setSingleShot(false);

    QObject::connect(&m_timer, &QTimer::timeout, [&]()
    {
        Visualizer::Common::fft_result result;
        if (m_queue.pop(result))
        {
            QVariantList list;
            for (int i = 0; i < Visualizer::Common::fftResultSize; ++i)
            {
                QVariantMap element;
                element["val"] = qreal(result[i]);
                list.append(element);
            }

            m_model.replaceList(list);;
        }
    });

    m_timer.start();
}
