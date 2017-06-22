/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_TEST__TCP_SERVER_TEST_CASE_H
#define NET_TEST__TCP_SERVER_TEST_CASE_H

#include <condition_variable>

#include "../../../net/notify-message.h"

namespace netty {
    namespace net {
        class ISocketService;
    }
    namespace test {
        class TcpServerTest {
        public:
            static void Run();

        private:
            static void recv_msg(std::shared_ptr<net::NotifyMessage> sspNM);
            static net::ISocketService     *s_ss;
            static common::MemPool         *m_mp;
        };
    }
}

#endif //NET_TEST__TCP_SERVER_TEST_CASE_H
