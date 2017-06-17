/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIXTCPCONNECTION_H
#define NET_CORE_POSIXTCPCONNECTION_H

#include "../../../../../../../common/common-def.h"
#include "../../../../../../common-def.h"
#include "../../socket-event-handler.h"
#include "stack/server-socket.h"
#include "../../../event-drivers/ievent-driver.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        class GCC_INTERNAL PosixTcpServerEventHandler : public SocketEventHandler {
        public:
            PosixTcpServerEventHandler(net_addr_t *nat, IEventDriver *ed, common::MemPool *memPool);
            ~PosixTcpServerEventHandler();

            bool HandleReadEvent() override;
            bool HandleWriteEvent() override;

        private:
            PosixTcpServerSocket *m_pSrvSocket;

            /**
             * 关联关系，外部创建者会释放，本类无需释放。
             */
            IEventDriver         *m_pEventDriver;
            /**
             * 关联关系，外部创建者会释放，本类无需释放。
             */
            common::MemPool      *m_pMemPool;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIXTCPCONNECTION_H
