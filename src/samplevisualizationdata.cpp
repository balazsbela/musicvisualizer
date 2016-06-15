#include "samplevisualizationdata.h"
#include "constants.h"

#include <QDebug>
#include <QColor>
#include <math.h>

#include <stdio.h>


//http://stackoverflow.com/questions/28948914/c-equivalent-to-this-glsl-functions

float fract (float f)
{
  float intpart;
  float fractpart = modf (f , &intpart);
  return fractpart;
}

void pack (float v, float* enc)
{
  enc [0] = fract (1.0f         * v);
  enc [1] = fract (255.0f       * v);
  enc [2] = fract (65025.0f     * v);
  enc [3] = fract (160581375.0f * v);

  enc [0] -= enc [1] * 1.0f/255.0f;
  enc [1] -= enc [2] * 1.0f/255.0f;
  enc [2] -= enc [3] * 1.0f/255.0f;
  enc [3] -= enc [3] * 0.0;

}

float unpack (const float* rgba)
{
  float dot = rgba [0] * 1.0f               +  rgba [1] * (1.0f / 255.0f)  +
              rgba [2] * (1.0f / 65025.0f)  +  rgba [3] * (1.0f / 160581375.0f);
  return dot;
}


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

                const qreal value = qreal(Visualizer::Common::sampleToFloat(result.data[i]));
                element["val"] = value;
                list.append(element);

                // The image is stored using a 32-bit ARGB format (0xAARRGGBB).

                qreal normalizedVal = (value + 1.0) / 2.0; // normalize to [0, 1) range
                float floats[4];
                pack(normalizedVal, floats);

                // Things go wrong when alpha is 0, so make sure to use the largest value for alpha

                const QColor color = QColor::fromRgbF(floats[3], 0 * floats[2], 0 * floats[1], floats[0]);
                m_bufferTextureProvider.setPixel(QPoint(i, 0), color);
            }

            m_model.replaceList(list);;
        }
    });

    m_timer.start();
}
