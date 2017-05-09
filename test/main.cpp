#include <iostream>

#include "../common/timer.h"
#include "../common/common-utils.h"
#include "../net/common-def.h"

using namespace common;
using namespace net;

int main() {
    /*************************************test timer**************************************/
    Timer timer;
    timer.Start();
    Timer::TimerCallback cb = [](void *ctx) {
        std::cout << "timer callback!" << CommonUtils::get_current_time().sec << std::endl;
    };

    for (int i = 0; i < 10; ++i) {
        Timer::Event ev(nullptr, &cb);
        auto eventId = timer.SubscribeEventAfter(uctime_t(3 * (i + 1), 0), ev);
    }

    getchar();

    return 0;
}
