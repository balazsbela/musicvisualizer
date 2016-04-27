#pragma once

#include "constants.h"

#include <fftw3.h>

#include <QObject>
#include <QQueue>
#include <QTimer>


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

    explicit FFTWrapper(Visualizer::Constants::queue_t& queue, QObject *parent = nullptr);
    ~FFTWrapper();


public slots:

    void stop();
    void start();
    void pullBuffer();

signals:

    void stopped();

private:
    const void calculate();


    fftw_plan m_plan;

    fftw_complex* m_in = nullptr;
    fftw_complex* m_out = nullptr;

    QQueue<freq_band_result_t> m_resultQueue;

    Visualizer::Constants::queue_t& m_queue;

    QTimer m_pullTimer;

};
