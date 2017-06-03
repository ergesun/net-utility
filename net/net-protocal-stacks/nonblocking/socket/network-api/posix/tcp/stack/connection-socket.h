/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCPCONNECTIONSOCKET_H
#define NET_CORE_POSIX_TCPCONNECTIONSOCKET_H

#include <stdexcept>
#include <sys/socket.h>

#include "../../../../../../../common-def.h"
#include "../../../socket-descriptor.h"

namespace netty {
    namespace net {
        /**
         * 当前只考虑了ipv4.
         */
        class PosixTcpConnectionSocket : public SocketDescriptor {
        public:
            PosixTcpConnectionSocket(net_addr_s nas) {
                m_local_addr = nas;
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

            ssize_t Write(void *buf, size_t nbytes);

            ssize_t Read(void *buf, size_t nbytes);

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

            /**
             * 获取内部的文件描述符。
             * @return
             */
            inline int GetSfd() {
                return m_sd;
            }

        protected:
            net_addr_s m_local_addr;

        private:
            volatile bool m_connected = false;
        };
    } // namespace net
} // namespace netty
#endif //NET_CORE_POSIX_TCPCONNECTIONSOCKET_H
