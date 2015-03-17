#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <string>

namespace smpl {

    class Channel{

        Channel(const Channel&) = delete;

        public:

            virtual std::string send() = 0;
            virtual std::string recv() = 0;

    };

}

#endif