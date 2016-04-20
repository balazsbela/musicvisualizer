#include "fftwrapper.h"

#include <math.h>
#include <algorithm>
#include <iostream>

#include <QDebug>


FFTWrapper::FFTWrapper()
{
    m_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * s_bufferSize);
    m_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * s_bufferSize);

    m_plan = fftw_plan_dft_1d(int(s_bufferSize), m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);
}


FFTWrapper::~FFTWrapper()
{
    fftw_destroy_plan(m_plan);
    fftw_free(m_in);
    fftw_free(m_out);
}


void FFTWrapper::feedBuffer(const char* buffer, unsigned size)
{
    if (!buffer || size == 0)
    {
        return;
    }

    // TODO: This is to limit the dropouts
    // Do the calculation on a different thread

    const unsigned nrChunks = std::min( int(size % s_bufferSize), 1);

    for (int i = 0; i < nrChunks; ++i)
    {

        for (int j = 0; j < s_bufferSize; ++j)
        {
            const unsigned index = nrChunks * s_bufferSize + j;

            m_in[j][0] = buffer[index]; // real part
            m_in[j][1] = 0; // imaginary part
        }

        calculate();
    }
}


const FFTWrapper::result_t& FFTWrapper::calculate()
{
    fftw_execute(m_plan);

    for ( int i = 0 ; i < s_nrOfSamples; ++i )
    {
                m_result[i] = sqrt( pow((m_out[i][0]),2) + pow((m_out[i][1]),2) );
                std::cout << m_result[i] << " ";
    }

    std::cout << std::endl << "_____________________________" << std::endl;
    return m_result;
}
