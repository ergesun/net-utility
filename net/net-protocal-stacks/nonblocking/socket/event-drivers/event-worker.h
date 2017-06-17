/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
#define NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H

#include "ievent-driver.h"
#include "../../../../common-def.h"
#include "../../../../../common/spin-lock.h"

namespace netty {
    namespace net {
        class EventWorker {
        public:
            EventWorker(uint32_t maxEvents, NonBlockingEventModel m);
            ~EventWorker();

            inline std::vector<NetEvent>* GetEventsContainer() {
                return &m_vEvents;
            }

            inline IEventDriver* GetDriver() {
                return m_pEventDriver;
            }

            inline common::spin_lock_t* GetSpinLock() {
                return &m_sl;
            }

        private:
            std::vector<NetEvent>   m_vEvents;
            IEventDriver           *m_pEventDriver;
            common::spin_lock_t     m_sl = UNLOCKED;
        };
    }  // namespace net
}  // namespace netty

#endif //NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
