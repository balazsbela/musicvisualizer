#include "samplevisualizationdata.h"
#include "constants.h"

SampleVisualizationData::SampleVisualizationData(Visualizer::Common::sample_queue_t& queue,
                                                 QObject* parent)
    : QObject(parent)
    , m_queue(queue)
{
    m_model.setParent(this);
    m_timer.setParent(this);

    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(16);
    m_timer.setSingleShot(false);

    QObject::connect(&m_timer, &QTimer::timeout, [&]()
    {
        Visualizer::Common::SampleEvent result;
        if (m_queue.pop(result))
        {
            QVariantList list;
            for (int i = 0; i < Visualizer::Common::nrSamples; ++i)
            {
                QVariantMap element;
                element["val"] = qreal(Visualizer::Common::sampleToFloat(result.data[i]));
                list.append(element);
            }

            m_model.replaceList(list);;
        }
    });

    m_timer.start();
}
