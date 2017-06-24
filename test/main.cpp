/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../common/timer.h"
#include "../common/common-utils.h"
#include "../net/common-def.h"

#include "thread-pool-test-case.h"
#include "timer-test-case.h"
#include "mem-pool-test-case.h"
#include "tcp/client/tcp-client-test-case.h"
#include "tcp/server/tcp-server-test-case.h"

using namespace netty::common;
using namespace netty::net;

using namespace netty::test;

int main() {
    std::cout << "/*********TESTCASE - 1: -->  thread pool test**********/" << std::endl;
    ThreadPoolTest::Run();

    std::cout << "\n\n/*********TESTCASE - 2: -->  simple memory pool test**********/" << std::endl;
    MemPoolTest::Run();

    std::cout << "\n\n/*********TESTCASE - 3: -->  timer test**********/" << std::endl;
    TimerTest::Run();

    std::cout << "\n\n/*********TESTCASE - 4: -->  simple tcp communication test**********/" << std::endl;
    TcpServerTest::Run();
    TcpClientTest::Run();

    getchar();

    return 0;
}
