#pragma once

#include <qglobal.h>

#include <boost/lockfree/queue.hpp>
#include <array>

namespace Visualizer {

namespace Common
{
    // They need to be more or less the same, otherwise the picture lags to the audio
    const unsigned fftTimerInterval = 20;
    const unsigned audioTimerInterval = 20;

    const unsigned sampleRate = 44100;

    // FFT Related

    const unsigned fftNrSamples = 1024;
    const unsigned fftResultSize = fftNrSamples / 2;
    const unsigned fftBufferSize = 1 * fftNrSamples;

    using sample_t = qint16;

    struct FFTEvent
    {
        std::array<sample_t, fftBufferSize> data = {0};
        unsigned nrElements = 0;
        unsigned nrChannels = 0;
    };

    using fft_queue_t = boost::lockfree::queue<FFTEvent, boost::lockfree::fixed_sized<false>>;
    using fft_result = std::array<double, fftResultSize>;
    using fft_result_queue_t = boost::lockfree::queue<fft_result, boost::lockfree::fixed_sized<false>>;


    // Sample queue, exposes audio data to other threads

    const unsigned nrSamples = 1000;
    using sample_array = std::array<sample_t, nrSamples>;

    struct SampleEvent
    {
        sample_array data = {0};
        unsigned nrElements = 0;
        unsigned nrChannels = 0;
    };

    using sample_queue_t = boost::lockfree::queue<SampleEvent, boost::lockfree::fixed_sized<false>>;


    static double sampleToFloat(qint16 value)
    {
        const qint16 PCMS16MaxValue     =  32767;
        const double PCMS16MaxAmplitude =  32768.0; // because minimum is -32768

        double f = double(value) / PCMS16MaxAmplitude;

        if (f > 1.0) f = 1.0;
        if (f < -1.0) f = -1.0;

        return f;
    }

}

}
