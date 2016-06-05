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
    const unsigned fftNrSamples = 1024;
    const unsigned fftResultSize = fftNrSamples / 2;
    const unsigned fftBufferSize = 1 * fftNrSamples;

    using sample_t = qint16;

    struct Event
    {
        std::array<sample_t, fftBufferSize> data = {0};
        unsigned nrElements = 0;
        unsigned nrChannels = 0;
    };

    using sample_queue_t = boost::lockfree::queue<Event, boost::lockfree::fixed_sized<false>>;
    using fft_result = std::array<double, fftResultSize>;
    using fft_result_queue_t = boost::lockfree::queue<fft_result, boost::lockfree::fixed_sized<false>>;
}

}
