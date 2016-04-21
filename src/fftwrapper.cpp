#include "fftwrapper.h"

#include <math.h>
#include <algorithm>
#include <iostream>

#include <QDebug>


FFTWrapper::FFTWrapper()
{
    m_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * s_fftInputSize);
    m_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * s_fftInputSize);

    m_plan = fftw_plan_dft_1d(int(s_fftInputSize), m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);
}


FFTWrapper::~FFTWrapper()
{
    fftw_destroy_plan(m_plan);
    fftw_free(m_in);
    fftw_free(m_out);
}


void FFTWrapper::feedBuffer(const char* audioBuffer, unsigned audioBufferSize)
{
    if (!audioBuffer || audioBufferSize < s_fftInputSize)
    {
        // Underrun
        return;
    }

    // Our buffer can be bigger, in case we can have multiple
    // sections of 512 samples in our buffer.
    // We collect a sample from each until the fft input buffer is populated

    const unsigned nrSections = audioBufferSize / s_fftInputSize;
    unsigned j = 0;

    for (int i = 0; i < audioBufferSize; i+= nrSections)
    {
        // If we jumped an uneven number of samples, we need to mind the channel count
        const bool isEven = nrSections % 2 == 0 || i == 0;

        // Real part
        m_in[j][0] = isEven ? (audioBuffer[i] / 2 + audioBuffer[i + 1] / 2) / s_fftInputSampleCount
                            : (audioBuffer[i - 1] / 2 + audioBuffer[i] / 2) / s_fftInputSampleCount;
        // Imaginary part
        m_in[j][1] = 0;

        if (j++ >= s_fftInputSize)
        {
            // Input data buffer is populated, ready to feed
            break;
        }
    }

    calculate();

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

    qDebug() << "_____________________________";

    // Normalize values

    for (int i = 0; i < 16; ++i)
    {
        rms[i] = float(rms[i]) / float(max);
        qDebug() << i << rms[i];
    }


    m_resultQueue.enqueue(std::move(rms));
}
