/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_H
#define NET_CORE_SOCKETAPI_SOCKET_H

namespace netty {
    namespace net {
        class SocketDescriptor {
        public:
            SocketDescriptor() {}
            SocketDescriptor(int sd) : m_sd(sd) {}

            /**
             * 获取内部的文件描述符。
             * @return
             */
            inline int GetSfd() {
                return m_sd;
            }

        protected:
            int m_sd;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_H
