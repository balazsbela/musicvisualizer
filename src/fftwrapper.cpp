#include "fftwrapper.h"

#include <math.h>
#include <algorithm>
#include <iostream>

#include <QtMath>
#include <QDebug>


FFTWrapper::FFTWrapper(Visualizer::Common::fft_queue_t& inQueue,
                       Visualizer::Common::fft_result_queue_t& outQueue,
                       QObject* parent)
    : QObject(parent),
      m_inQueue(inQueue),
      m_outQueue(outQueue)
{
    m_pullTimer.setParent(this);
}


FFTWrapper::~FFTWrapper()
{
    fftw_destroy_plan(m_plan);

    if (m_out)
    {
        fftw_free(m_out);
        m_out = nullptr;
    }

    if (m_in)
    {
        fftw_free(m_in);
        m_in = nullptr;
    }
}


void FFTWrapper::start()
{
    calculateWindow();

    m_in = fftw_alloc_complex(s_fftInputSize);
    m_out = fftw_alloc_complex(s_fftInputSize);
    m_plan = fftw_plan_dft_1d(s_fftInputSize, m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);

    m_pullTimer.setTimerType(Qt::PreciseTimer);
    m_pullTimer.setInterval(Visualizer::Common::fftTimerInterval);
    m_pullTimer.setSingleShot(false);
    QObject::connect(&m_pullTimer, &QTimer::timeout, this, &FFTWrapper::pullBuffer);

    m_pullTimer.start();
}


void FFTWrapper::stop()
{
    m_pullTimer.stop();
    emit stopped();
}


void FFTWrapper::calculateWindow()
{
    for (int i = 0; i < s_fftInputSize; ++i)
    {
        switch (m_windowFunction) {
        case NoWindow:
            m_window[i] = 1.0;
            break;
        case HannWindow:
            m_window[i] = 0.5 * (1 - qCos((2 * M_PI * i) / float(s_fftInputSize - 1)));
            break;
        default:
            Q_ASSERT(false);
        }
    }
}


void FFTWrapper::pullBuffer()
{
    Visualizer::Common::FFTEvent event;
    if (m_inQueue.pop(event))
    {
        // Our buffer can be bigger, in case we can have multiple
        // sections of nrSamples samples in our buffer.
        // We collect a sample from each until the fft input buffer is populated

        const unsigned nrSections = event.nrElements / s_fftInputSize;
        const bool isEven = nrSections % 2 == 0;
        unsigned j = 0;

        if (event.nrChannels == 1)
        {
            for (int i = 0; i < event.nrElements; i += nrSections)
            {
                m_in[j][0] = Visualizer::Common::sampleToFloat(event.data[i]) * m_window[j];
                m_in[j][1] = 0;

                if (++j >= s_fftInputSize)
                {
                    // Input data buffer is populated, ready to feed
                    break;
                }
            }
        }
        else
        {
            const unsigned lastIndex = isEven ? event.nrElements - 1 : event.nrElements;
            for (int i = 0; i < lastIndex; i+= nrSections)
            {
                // If we jumped an uneven number of samples, we need to mind the channel count

                // Real part
                m_in[j][0] = isEven || i == 0 ? (Visualizer::Common::sampleToFloat(event.data[i]) + Visualizer::Common::sampleToFloat(event.data[i + 1])) / 2.0f
                                              : (Visualizer::Common::sampleToFloat(event.data[i - 1]) + Visualizer::Common::sampleToFloat(event.data[i])) / 2.0f;

                // Imaginary part
                m_in[j][1] = 0.0;

                if (++j >= s_fftInputSize)
                {
                    // Input data buffer is populated, ready to feed
                    break;
                }
            }
        }

        calculate();
    }

}


void FFTWrapper::calculate()
{
    fftw_execute(m_plan);

    Visualizer::Common::fft_result spectrum;

//    qDebug() << "_________________________________________________";

    double maxMagnitude = 0.0;

    for (int i = 2; i <= s_fftInputSampleCount; ++i )
    {
        spectrum[i] = sqrt( m_out[i][0] * m_out[i][0] + m_out[i][1] * m_out[i][1] );
        maxMagnitude = std::max(maxMagnitude, spectrum[i]);
    }

//    const float binFrequencySize =  Visualizer::Constants::sampleRate / s_fftInputSize;
    for (int i = 2; i <= s_fftInputSampleCount; ++i)
    {
//        qDebug() << i << " " << "Frequency: " << float(i) * binFrequencySize << "-" << float(i+1) * binFrequencySize << " " << spectrum[i] / maxMagnitude;
        spectrum[i] = spectrum[i] / maxMagnitude;
    }

    m_outQueue.push(spectrum);
}
