/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <memory.h>
#include <netinet/in.h>

#include "../../../../../../../../common/common-utils.h"
#include "server-socket.h"

namespace netty {
    namespace net {
        bool PosixTcpServerSocket::Bind() {
            struct sockaddr_in serv_addr;
            bzero(&serv_addr, sizeof(serv_addr));
            // 协议族ipv4
            serv_addr.sin_family = AF_INET;
            // 0.0.0.0，监听本机的内、外所有ip。
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            serv_addr.sin_port = htons(m_local_addr.port);

            // 讲socket fd绑定到ip:port
            auto rc = (0 == bind(m_fd, (sockaddr *) &serv_addr, sizeof(serv_addr)));
            if (!rc) {
                fprintf(stderr, "Error bind %d - %s\n", errno, strerror(errno));
            }

            return rc;
        }

        bool PosixTcpServerSocket::Listen() {
            auto rc = (0 == listen(m_fd, m_max_listen_conns));
            if (!rc) {
                fprintf(stderr, "Error listen %d - %s\n", errno, strerror(errno));
            }

            return rc;
        }

        bool PosixTcpServerSocket::SetPortReuse() {
            auto rc = (0 == setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &m_local_addr.port, sizeof(int)));
            if (!rc) {
                fprintf(stderr, "Error SetPortReuse %d - %s\n", errno, strerror(errno));
            }

            return rc;
        }

        int PosixTcpServerSocket::Accept4(__SOCKADDR_ARG __addr, socklen_t *__addr_len, int __flags) {
            return accept4(m_fd, __addr, __addr_len, __flags);
        }
    } // namespace net
} // namespace netty
