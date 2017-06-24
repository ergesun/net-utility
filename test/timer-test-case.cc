/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include <atomic>
#include <unistd.h>

#include "../common/timer.h"

#include "../common/common-utils.h"
#include "timer-test-case.h"

using namespace std;

namespace netty {
    namespace test {
        void TimerTest::Run() {
            common::Timer timer;
            timer.Start();
            int test_cnt = 5;
            std::atomic<int> backs_cnt{0};
            common::Timer::TimerCallback cb = [&backs_cnt](void *ctx) {
                std::cout << "timer callback!" << common::CommonUtils::GetCurrentTime().sec << std::endl;
                backs_cnt++;
            };

            for (int i = 0; i < test_cnt; ++i) {
                common::Timer::Event ev(nullptr, &cb);
                auto eventId = timer.SubscribeEventAfter(common::uctime_t(1 * (i + 1), 0), ev);
            }

            while (test_cnt != backs_cnt.load()) {
                sleep(3);
            }
        }
    }
}
