#include "fftwrapper.h"

#include <math.h>
#include <algorithm>
#include <iostream>

#include <QDebug>


FFTWrapper::FFTWrapper(Visualizer::Constants::sample_queue_t& inQueue,
                       Visualizer::Constants::fft_result_queue_t& outQueue,
                       QObject* parent)
    : QObject(parent)
    , m_inQueue(inQueue)
    , m_outQueue(outQueue)
{
    m_in = fftw_alloc_complex(s_fftInputSize);
    m_out = fftw_alloc_complex(s_fftInputSize);

    m_plan = fftw_plan_dft_1d(int(s_fftInputSize), m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);

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
    m_pullTimer.setInterval(0);
    m_pullTimer.setSingleShot(false);
    QObject::connect(&m_pullTimer, &QTimer::timeout, this, &FFTWrapper::pullBuffer);

    m_pullTimer.start();
}


void FFTWrapper::stop()
{
    m_pullTimer.stop();
    emit stopped();
}


void FFTWrapper::pullBuffer()
{
    Visualizer::Constants::Event event;

    while (m_inQueue.pop(event))
    {
        // Our buffer can be bigger, in case we can have multiple
        // sections of 512 samples in our buffer.
        // We collect a sample from each until the fft input buffer is populated

        const unsigned nrSections = event.nrElements / s_fftInputSize;
        unsigned j = 0;

        for (int i = 0; i < event.nrElements; i+= nrSections)
        {
            // If we jumped an uneven number of samples, we need to mind the channel count
            const bool isEven = nrSections % 2 == 0 || i == 0;

            // Real part
            m_in[j][0] = isEven ? (event.data[i] / 2 + event.data[i + 1] / 2) / s_fftInputSampleCount
                                : (event.data[i - 1] / 2 + event.data[i] / 2) / s_fftInputSampleCount;

            // Imaginary part
            m_in[j][1] = 0.0;

            if (j++ >= s_fftInputSize - 1)
            {
                // Input data buffer is populated, ready to feed
                break;
            }
        }

        calculate();
    }

}


const void FFTWrapper::calculate()
{
    fftw_execute(m_plan);

    spectrum_result_t spectrum;

    for ( int i = 0 ; i < s_fftInputSampleCount; ++i )
    {
        spectrum[i] = sqrt( pow(m_out[i][0],2) + pow(m_out[i][1],2) );
    }

    freq_band_result_t rms;
    float max = 0;

    for(int i = 0; i < s_fftNumberOfBands; ++i)
    {
        unsigned rootSum = 0;
        for (int index = s_xscale[i]; index < s_xscale[i + 1]; index++)
        {
            rootSum += pow(spectrum[index], 2);
        }

        const unsigned numberOfValues = s_xscale[i + 1] - s_xscale[i];
        rms[i] = sqrt(rootSum/numberOfValues);

        if (rms[i] > max)
        {
            max = rms[i];
        }
    }

   // qDebug() << "_____________________________";

    // Normalize values

    for (int i = 0; i < 16; ++i)
    {
        rms[i] = max > 0 ? float(rms[i]) / float(max) : 0;
        // qDebug() << i << rms[i];
    }


    m_outQueue.push(std::move(rms));
}
