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

    static const unsigned s_fftInputSize = Visualizer::Common::fftNrSamples;
    static const unsigned s_fftInputSampleCount =  Visualizer::Common::fftResultSize;

    static const unsigned s_fftNumberOfBands = 16;
    const unsigned s_xscale[s_fftNumberOfBands + 1] = {0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 74, 101, 137, 187, 255};

    using spectrum_result_t = std::array<double, s_fftInputSampleCount>;
    using freq_band_result_t = std::array<double, s_fftNumberOfBands>;

    explicit FFTWrapper(Visualizer::Common::fft_queue_t& inQueue,
                        Visualizer::Common::fft_result_queue_t& outQueue,
                        QObject *parent = nullptr);
    ~FFTWrapper();


public slots:

    void stop();
    void start();
    void pullBuffer();

signals:

    void stopped();

private:

    void calculate();
    void calculateWindow();

    fftw_plan m_plan;

    fftw_complex* m_in = nullptr;
    fftw_complex* m_out = nullptr;

    Visualizer::Common::fft_queue_t& m_inQueue;
    Visualizer::Common::fft_result_queue_t& m_outQueue;

    QTimer m_pullTimer;

    std::array<double, s_fftInputSize> m_window;

    enum WindowFunction {
        NoWindow,
        HannWindow
    };

    WindowFunction m_windowFunction = HannWindow;

};
