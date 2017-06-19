/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
#define NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H

#include "../../ievent-handler.h"
#include "file-descriptor.h"
#include "../../../../../common/common-def.h"
#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {
        class AFileEventHandler : public IEventHandler {
        public:
            AFileEventHandler() = default;
            AFileEventHandler(FileDescriptor *socketDesc) : m_socketDesc(socketDesc) {}

            virtual ~AFileEventHandler() = default;
            inline FileDescriptor* GetSocketDescriptor() {
                return m_socketDesc;
            }

            virtual ANetStackMessageWorker* GetStackMsgWorker() = 0;

        protected:
            inline void SetSocketDescriptor(FileDescriptor *psd) {
                m_socketDesc = psd;
            }

        private:
            FileDescriptor *m_socketDesc;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
