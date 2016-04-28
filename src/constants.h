#pragma once

#include <qglobal.h>

#include <boost/lockfree/queue.hpp>
#include <array>

namespace Visualizer {

namespace Constants
{

    const unsigned sampleRate = 48000;
    const unsigned bufferSize = 10 * 512;
    const unsigned fftResultSize = 256;

    struct Event
    {
        std::array<int, bufferSize> data;
        unsigned nrElements = 0;
    };

    using sample_queue_t = boost::lockfree::queue<Event, boost::lockfree::fixed_sized<false>>;
    using fft_result = std::array<double, fftResultSize>;
    using fft_result_queue_t = boost::lockfree::queue<fft_result, boost::lockfree::fixed_sized<false>>;
}

}
