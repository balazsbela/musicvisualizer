#pragma once

#include <fftw3.h>

#include <QObject>
#include <QQueue>


class FFTWrapper : public QObject
{
Q_OBJECT;

public:

    static const unsigned s_fftInputSize = 512;
    static const unsigned s_fftInputSampleCount = s_fftInputSize / 2;
    static const unsigned s_fftNumberOfBands = 16;
    const unsigned s_xscale[s_fftNumberOfBands + 1] = {0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 74, 101, 137, 187, 255};

    using spectrum_result_t = std::array<long, s_fftInputSampleCount>;
    using freq_band_result_t = std::array<float, s_fftNumberOfBands>;

    FFTWrapper();
    ~FFTWrapper();

    void feedBuffer(const char* buffer, unsigned size);
    const void calculate();


private:

    fftw_complex* m_in;
    fftw_complex* m_out;
    fftw_plan m_plan;

    QQueue<freq_band_result_t> m_resultQueue;

};
