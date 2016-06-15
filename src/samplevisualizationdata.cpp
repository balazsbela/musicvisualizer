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

                qreal normalizedVal = (value + 1.0) / 2.0;
                float floatval = normalizedVal;

                float floats[4];
                pack(floatval, floats);

//                qDebug() << "____________________";
//                qDebug() << normalizedVal << value << (normalizedVal * 2.0) - 1.0;
//                qDebug() << floats[0] << floats[1] << floats[2] << floats[3];

//                unsigned red = floats[0] * 255.0f;
//                unsigned green = floats[1] * 255.0f;
//                unsigned blue = floats[2] * 255.0f;
//                unsigned alpha = floats[3] * 255.0f;

//                qDebug() << red << green << blue << alpha;

//                float fred = red / 255.0f;
//                float fgreen = green / 255.0f;
//                float fblue = blue / 255.0f;
//                float falpha = alpha / 255.0f;

//                float testfloats[4];

//                testfloats[0] = fred;
//                testfloats[1] = fgreen;
//                testfloats[2] = fblue;
//                testfloats[3] = falpha;

//                qDebug() << testfloats[0] << testfloats[1] << testfloats[2] << testfloats[3];

//                qDebug() << unpack(testfloats) << floatval;

//                unsigned intval = 0;//0x880000FF;
//                memcpy(&intval, &floatval, sizeof(unsigned));

//                unsigned blue   =   intval & 0x000000FF;
//                unsigned green =   (intval & 0x0000FF00) >> 8;
//                unsigned red  =    (intval & 0x00FF0000) >> 16;
//                unsigned alpha =   (intval & 0xFF000000) >> 24;


//                qDebug() << red << green << blue << alpha;

//                float fred,fblue, fgreen, falpha;

//                memcpy(&fred, &red, sizeof(float));
//                memcpy(&fblue, &blue, sizeof(float));
//                memcpy(&fgreen, &green, sizeof(float));
//                memcpy(&falpha, &alpha, sizeof(float));

//                float colors[4];
//                colors[0] = red / 255.0f;
//                colors[1] = green / 255.0f;
//                colors[2] = blue / 255.0f;
//                colors[3] = alpha / 255.0f;

//                float result = unpack(colors);

//                qDebug() << result << "vs" << floatval;


//                qDebug() << fred << fgreen << fblue << falpha;


//                 // Verify calculation

//                unsigned rgbaint = (alpha << 24) + (red << 16) + (green << 8) + blue;
//                float newFloatVal = 0;
//                memcpy(&newFloatVal, &rgbaint, sizeof(float));
//                Q_ASSERT(intval == rgbaint);
//                Q_ASSERT(abs(newFloatVal - floatval < 0.000001));

                // Things go wrong when alpha is 0, so make sure to use the largest value for alpha

                const QColor color = QColor::fromRgbF(floats[3], 0 * floats[2], 0 * floats[1], floats[0]);
                m_bufferTextureProvider.setPixel(QPoint(i, 0), color);
            }

            m_model.replaceList(list);;
        }
    });

    m_timer.start();
}
