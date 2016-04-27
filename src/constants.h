#pragma once

#include <boost/lockfree/queue.hpp>

namespace Visualizer {

namespace Constants
{

    const unsigned bufferSize = 10 * 512;
    const unsigned queueSize = 100;

    struct Event
    {
        std::array<int, bufferSize> data;
        unsigned nrElements = 0;
    };

    using queue_t = boost::lockfree::queue<Event>;

}

}
