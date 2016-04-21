#pragma once


#include <fftw3.h>


class FFTWrapper
{

public:

    static const unsigned s_bufferSize = 512;
    static const unsigned s_nrOfSamples = s_bufferSize / 2;

    using result_t = long int[s_nrOfSamples];

    FFTWrapper();
    ~FFTWrapper();

    void feedBuffer(const char* buffer, unsigned size);

    const result_t& calculate();


private:

    fftw_complex* m_in;
    fftw_complex* m_out;
    fftw_plan m_plan;

    result_t m_result;

};
