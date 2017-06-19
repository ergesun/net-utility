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
                return &m_vEpollEvents;
            }

            inline IEventDriver* GetDriver() {
                return m_pEventDriver;
            }

            inline common::spin_lock_t* GetSpinLock() {
                return &m_sl;
            }

            inline void AddExternalEvent(NetEvent ne) {
                common::SpinLock l(&m_slEE);
                m_lExternalEvents.push_back(ne);
            }

            inline std::list<NetEvent> GetExternalEvents() {
                common::SpinLock l(&m_slEE);
                std::list<NetEvent> tmp;
                tmp.swap(m_lExternalEvents);

                return std::move(tmp);
            }

            void Wakeup();

        private:
            std::vector<NetEvent>   m_vEpollEvents;
            IEventDriver           *m_pEventDriver;
            common::spin_lock_t     m_sl = UNLOCKED;

            common::spin_lock_t     m_slEE = UNLOCKED;
            std::list<NetEvent>     m_lExternalEvents;
            int                     m_notifySendFd;
            int                     m_notifyRecvFd;
            AFileEventHandler      *m_pLocalReadEventHandler;
        };
    }  // namespace net
}  // namespace netty

#endif //NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
