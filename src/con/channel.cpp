/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/con/channel.h"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <sstream>
#include "common/sys/error.h"

namespace cxx {
    namespace con {

        spinlock::spinlock()
            : state_(Unlocked)
        {
        }

        void spinlock::acquire()
        {
            while(state_.xchg(Locked) == Locked) {
                // busy wait
            }
        }

        void spinlock::release()
        {
            state_.store(Unlocked);
        }

    }
}
