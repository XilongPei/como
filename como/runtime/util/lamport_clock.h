//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#ifndef __LAMPORT_CLOCK_LAMPORT_CLOCK_H__
#define __LAMPORT_CLOCK_LAMPORT_CLOCK_H__

#include <atomic>
#include "comotypes.h"

namespace como {

class LamportClock {
public:
    LamportClock()
        : time_(0)
    {}

    /**
     * Get current Lamport timestamp.
     *
     * @return Long value.
     */
    Long get_time() const {
        return time_.load();
    }

    /**
     * Handle local event.
     * Increment timer and return the new value.
     *
     * @return Long value;
     */
    Long local_event() {
        return time_.fetch_add(1);
    }

    /**
     * Handle send event.
     * Increment local time and return the new value.
     *
     * @return Long value;
     */
    Long send_event() {
        return time_.fetch_add(1);
    }

    /**
     * Handle receive event.
     * Receive sender's time and set the local time to the maximum between
     * received and local time plus 1.
     *
     * @param received_time Sender's time.
     * @return Long value;
     */
    Long receive_event(Long received_time) {
        while (true) {
            Long cur = get_time();

            // If received time is old, do nothing.
            if (received_time < cur) {
                return cur;
            }

            // Ensure that local timer is at least one ahead.
            if (! time_.compare_exchange_strong(cur, received_time + 1)) {
                continue;
            }
            else {
                break;
            }
        }

        return received_time + 1;
    }

private:
    std::atomic<Long> time_;
};

} // namespace como

#endif //__LAMPORT_CLOCK_LAMPORT_CLOCK_H__
