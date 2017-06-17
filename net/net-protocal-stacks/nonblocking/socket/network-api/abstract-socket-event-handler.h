/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
#define NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H

#include "../../ievent-handler.h"
#include "socket-descriptor.h"
#include "../../../../../common/common-def.h"
#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {

        class ASocketEventHandler : public IEventHandler {
        public:
            ASocketEventHandler() = default;
            ASocketEventHandler(SocketDescriptor *socketDesc) : m_socketDesc(socketDesc) {}

            virtual ~ASocketEventHandler() = default;
            inline SocketDescriptor* GetSocketDescriptor() {
                return m_socketDesc;
            }

            virtual ANetStackMessageWorker* GetStackMsgWorker() = 0;

        protected:
            inline void SetSocketDescriptor(SocketDescriptor *psd) {
                m_socketDesc = psd;
            }

        private:
            SocketDescriptor *m_socketDesc;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
