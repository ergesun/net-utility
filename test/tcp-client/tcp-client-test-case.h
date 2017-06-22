/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_TEST_TCP_CLIENT_TEST_CASE_H
#define NET_TEST_TCP_CLIENT_TEST_CASE_H

#include "../net/notify-message.h"

namespace netty {
    namespace test {
        class TcpClientTest {
        public:
            static void Run();

        private:
            static void recv_msg(std::shared_ptr<net::NotifyMessage> sspNM);
        };
    }
}

#endif //NET_TEST_TCP_CLIENT_TEST_CASE_H
