/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIXTCPCONNECTION_H
#define NET_CORE_POSIXTCPCONNECTION_H

#include "../../../../../../../common/common-def.h"
#include "../../../../../../common-def.h"
#include "../../abstract-file-event-handler.h"
#include "stack/server-socket.h"
#include "../../../event-drivers/ievent-driver.h"
#include "../../abstract-event-manager.h"
#include "../../../../../../notify-message.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        class GCC_INTERNAL PosixTcpServerEventHandler : public AFileEventHandler {
        public:
            PosixTcpServerEventHandler(EventWorker *ew, net_addr_t *nat, ConnectHandler onConnect, common::MemPool *memPool,
                                       NotifyMessageCallbackHandler msgCallbackHandler);
            ~PosixTcpServerEventHandler();

            bool HandleReadEvent() override;
            bool HandleWriteEvent() override;

            ANetStackMessageWorker *GetStackMsgWorker() override;

        private:
            inline void handle_message(NotifyMessage* nm);

        private:
            PosixTcpServerSocket   *m_pSrvSocket;
            ConnectHandler          m_onConnect;
            /**
             * 关联关系，外部创建者会释放，本类无需释放。
             */
            common::MemPool        *m_pMemPool;
            NotifyMessageCallbackHandler      m_msgCallbackHandler;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIXTCPCONNECTION_H
