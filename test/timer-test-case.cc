/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../common/timer.h"

#include "timer-test-case.h"
#include "../common/common-utils.h"

using namespace std;

namespace netty {
    namespace test {
        void TimerTest::Run() {
            common::Timer timer;
            timer.Start();
            common::Timer::TimerCallback cb = [](void *ctx) {
                std::cout << "timer callback!" << common::CommonUtils::get_current_time().sec << std::endl;
            };

            for (int i = 0; i < 10; ++i) {
                common::Timer::Event ev(nullptr, &cb);
                auto eventId = timer.SubscribeEventAfter(common::uctime_t(3 * (i + 1), 0), ev);
            }
        }
    }
}
