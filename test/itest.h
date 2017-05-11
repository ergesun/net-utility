//
// Created by sunchao31 on 17-5-11.
//

#ifndef NET_TEST_ITEST_H
#define NET_TEST_ITEST_H
namespace netty {
    namespace test {
        class ITest {
        public:
            ~ITest() {}

            virtual void Run() = 0;
        };
    }
}
#endif //NET_TEST_ITEST_H
