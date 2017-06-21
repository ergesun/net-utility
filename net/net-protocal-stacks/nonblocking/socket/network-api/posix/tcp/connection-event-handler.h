/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_CONNECTION_EVENT_HANDLER_H
#define NET_CORE_POSIX_TCP_CONNECTION_EVENT_HANDLER_H

#include "../../../../../../../common/common-def.h"
#include "../../abstract-file-event-handler.h"
#include "stack/connection-socket.h"
#include "net-stack-worker.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        class GCC_INTERNAL PosixTcpConnectionEventHandler : public AFileEventHandler {
        public:
            PosixTcpConnectionEventHandler(net_addr_t &peerAddr, int sfd, common::MemPool *memPool, NotifyMessageCallbackHandler msgCallbackHandler);
            ~PosixTcpConnectionEventHandler();

            bool HandleReadEvent() override;
            bool HandleWriteEvent() override;

            ANetStackMessageWorker *GetStackMsgWorker() override;

        private:
            PosixTcpClientSocket   *m_pClientSocket = nullptr;
            PosixTcpNetStackWorker *m_pNetStackWorker = nullptr;
            common::MemPool        *m_pMemPool;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIX_TCP_CONNECTION_EVENT_HANDLER_H
