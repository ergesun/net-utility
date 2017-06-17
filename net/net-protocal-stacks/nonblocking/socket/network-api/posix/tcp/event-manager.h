/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NB_SOCKET_NA_POSIX_TCP_EVENTMANAGER_H
#define NET_CORE_NB_SOCKET_NA_POSIX_TCP_EVENTMANAGER_H

#include <thread>
#include "../../../../../../common-def.h"

#include "../../abstract-event-manager.h"
#include "../../../event-drivers/ievent-driver.h"
#include "../../../event-drivers/event-worker.h"
#include "../../../../../../../common/spin-lock.h"

namespace netty {
    namespace net {
        class PosixTcpEventManager : public AEventManager {
        public:
            PosixTcpEventManager(net_addr_t *nat, common::MemPool *memPool, uint32_t maxEvents, uint32_t connWorkersCnt) :
                AEventManager(memPool, maxEvents), m_pNat(nat), m_iConnWorkersCnt(connWorkersCnt) {}

            ~PosixTcpEventManager();

            bool Start(NonBlockingEventModel m) override;
            bool Stop() override;

            int AddEvent(SocketEventHandler *socketEventHandler, int cur_mask, int mask) override;

        private:
            void worker_loop(EventWorker *ew, bool isServer);

        private:
            uint32_t                                           m_iConnWorkersCnt;
            int32_t                                            m_iCurWorkerIdx = -1;
            net_addr_t                                        *m_pNat;
            bool                                               m_bStopped = true;
            SocketEventHandler                                *m_pServerEventHandler = nullptr;
            std::pair<std::thread*, EventWorker*>              m_pListenWorkerEventLoopCtx;
            std::vector<std::pair<std::thread*, EventWorker*>> m_vConnsWorkerEventLoopCtxs;
            common::spin_lock_t                                m_slSelectEvents = UNLOCKED;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_NB_SOCKET_NA_POSIX_TCP_EVENTMANAGER_H
