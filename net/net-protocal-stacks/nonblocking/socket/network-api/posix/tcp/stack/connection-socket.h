/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCPCONNECTIONSOCKET_H
#define NET_CORE_POSIX_TCPCONNECTIONSOCKET_H

#include <stdexcept>
#include <sys/socket.h>

#include "../../../../../../../common-def.h"
#include "../../../file-descriptor.h"

namespace netty {
    namespace net {
        /**
         * 当前只考虑了ipv4.
         */
        class GCC_INTERNAL PosixTcpConnectionSocket : public FileDescriptor {
        public:
            PosixTcpConnectionSocket() = default;
            PosixTcpConnectionSocket(net_addr_t peerAddr) {
                m_real_peer = net_peer_info_t(peerAddr, SocketProtocal::Tcp);
            }

            PosixTcpConnectionSocket(net_addr_t peerAddr, int sfd) {
                m_real_peer = net_peer_info_t(peerAddr, SocketProtocal::Tcp);
                m_fd = sfd;
            }

            virtual ~PosixTcpConnectionSocket() {}

            /* basic interfaces */
            /**
             * 成功返回0,失败-1。
             * @param nas
             * @return
             */
            bool Socket();

            /**
             * 连接目标地址。
             * @param timeout 超时时间。如果为空则为阻塞式的默认行为。
             * @return
             */
            bool Connect(struct timeval *timeout);

            ssize_t Write(void *buf, size_t nbytes, int &err);

            ssize_t Read(void *buf, size_t nbytes, int &err);

            /* sock opts interfaces */
            bool SetSndBufferSize(int nbytes) throw(std::runtime_error);

            bool SetRcvBufferSize(int nbytes) throw(std::runtime_error);

            /**
             * disable nagle algorithm.
             * @param ok
             * @return
             */
            bool SetNoDelay(bool ok);

            bool SetSoLinger(bool ok, int seconds);

            bool SetKeepalive(bool ok);

            bool SetAddrReuse(bool ok);

            /**
             * 只是对setsockopt的包装以支持个性化通用配置。
             * @return
             */
            int SetSockOpt(int __level, int __optname,
                           const void *__optval, socklen_t __optlen);

            int Close();

        private:
            volatile bool m_connected = false;
        };

        typedef PosixTcpConnectionSocket PosixTcpClientSocket;
    } // namespace net
} // namespace netty
#endif //NET_CORE_POSIX_TCPCONNECTIONSOCKET_H
