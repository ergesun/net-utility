/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_H
#define NET_CORE_SOCKETAPI_SOCKET_H

#include "../../../../common-def.h"

namespace netty {
    namespace net {
        class FileDescriptor {
        public:
            FileDescriptor() = default;
            FileDescriptor(int sd, net_peer_info_t peer) : m_fd(sd), m_peer(peer) {}

            /**
             * 获取内部的文件描述符。
             * @return
             */
            inline int GetFd() {
                return m_fd;
            }

            /**
             * 获取文件描述符对应的端点信息。
             * @return
             */
            inline net_peer_info_t GetPeerInfo() {
                return m_peer;
            }

        protected:
            int               m_fd = -1;
            net_peer_info_t   m_peer;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_H
