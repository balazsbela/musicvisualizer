#include "fftvisualizationdata.h"
#include "constants.h"

FFTVisualizationData::FFTVisualizationData(Visualizer::Common::fft_result_queue_t& queue,
                                           QObject* parent)
    : QObject(parent)
    , m_queue(queue)
{
    m_model.setParent(this);
    m_timer.setParent(this);

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
