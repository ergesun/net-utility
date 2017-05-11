/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCPSERVERSOCKET_H
#define NET_CORE_POSIX_TCPSERVERSOCKET_H

#include <stdexcept>

#include "connection-socket.h"

namespace netty {
    namespace net {
        class PosixTcpServerSocket : public PosixTcpConnectionSocket {
        public:
            PosixTcpServerSocket(net_addr_s &nas, int max_conns) : PosixTcpConnectionSocket(nas),
                                                                   m_max_listen_conns(max_conns) {}

            /* basic interfaces */
            inline bool Bind();

            inline bool Listen();

            inline int Accept4(__SOCKADDR_ARG __addr, socklen_t *__addr_len, int __flags = SOCK_NONBLOCK);

            /* sock opts interfaces */
            inline bool SetPortReuse() throw(std::runtime_error);

        private:
            inline bool Connect() {
                throw new std::runtime_error("server socket doesn't support connect api.");
            }

        private:
            int m_max_listen_conns;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIX_TCPSERVERSOCKET_H
