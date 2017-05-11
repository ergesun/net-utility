//
// Created by sunchao31 on 17-5-11.
//

#ifndef NET_TEST_THREADPOOLTESTCASE_H
#define NET_TEST_THREADPOOLTESTCASE_H

#include "itest.h"

namespace netty {
    namespace test {
        class ThreadPoolTest : public ITest {
        public:
            virtual void Run() override;
        };
    }
}

#endif //NET_TEST_THREADPOOLTESTCASE_H
