/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <memory.h>
#include <netinet/in.h>

#include "../../../../../../../common/common-utils.h"
#include "server-socket.h"

namespace netty {
    namespace net {
        inline bool PosixTcpServerSocket::Bind() {
            common::CommonUtils::set_nonblocking(m_sd);

            struct sockaddr_in serv_addr;
            bzero(&serv_addr, sizeof(serv_addr));
            // 协议族ipv4
            serv_addr.sin_family = AF_INET;
            // 0.0.0.0，监听本机的内、外所有ip。
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            serv_addr.sin_port = htons(m_local_addr.port);

            // 讲socket fd绑定到ip:port
            return 0 == bind(m_sd, (sockaddr *) &serv_addr, sizeof(serv_addr));
        }

        bool PosixTcpServerSocket::Listen() {
            return 0 == listen(m_sd, m_max_listen_conns);
        }

        bool PosixTcpServerSocket::SetPortReuse() throw(std::runtime_error) {
            return 0 == setsockopt(m_sd, SOL_SOCKET, SO_REUSEPORT, &m_local_addr.port, sizeof(int));
        }

        int PosixTcpServerSocket::Accept4(__SOCKADDR_ARG __addr, socklen_t *__addr_len, int __flags) {
            struct sockaddr_in client_addr;
            socklen_t sock_len = sizeof(struct sockaddr_in);

            return accept4(m_sd, __addr, __addr_len, __flags);
        }
    } // namespace net
} // namespace netty
