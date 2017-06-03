/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
#define NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H

#include "../../ievent-handler.h"
#include "socket-descriptor.h"

namespace netty {
    namespace net {
        class SocketEventHandler : public IEventHandler {
        public:
            SocketEventHandler() = default;

            inline SocketDescriptor GetSocketDescriptor() {
                return m_socketDesc;
            }

        private:
            SocketDescriptor m_socketDesc;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
