/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
#define NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H

#include <set>
#include <mutex>
#include <list>

#include "ievent-driver.h"
#include "../../../../common-def.h"
#include "../../../../../common/spin-lock.h"

namespace netty {
    namespace net {
        /**
         * 事件管理器的封装。需要通过GetInternalEvent和GetExternalEvents两个函数才能获取所有事件。
         */
        class EventWorker {
        public:
            EventWorker(uint32_t maxEvents, NonBlockingEventModel m);
            ~EventWorker();

            inline std::vector<NetEvent>* GetEventsContainer() {
                return &m_vDriverInternalEvents;
            }

            inline int32_t GetInternalEvent(std::vector<NetEvent> *events, struct timeval *tp) {
                return m_pEventDriver->EventWait(events, tp);
            }

            inline int32_t AddEvent(AFileEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) {
                common::SpinLock l(&m_slDriver);
                m_pEventDriver->AddEvent(socketEventHandler, cur_mask, mask);
            }

            inline int32_t DeleteHandler(AFileEventHandler *socketEventHandler) {
                {
                    common::SpinLock l(&m_slDriver);
                    m_pEventDriver->DeleteHandler(socketEventHandler);
                }
                {
                    std::unique_lock<std::mutex> l(m_mtxPendingDeleteEHLock);
                    m_pendingDeleteEventHandlers.insert(socketEventHandler);
                }
            }

            inline std::set<AFileEventHandler*> GetPendingDeleteEventHandlers() {
                std::unique_lock<std::mutex> l(m_mtxPendingDeleteEHLock);
                std::set<AFileEventHandler*> tmp;
                m_pendingDeleteEventHandlers.swap(tmp);
                return std::move(tmp);
            }

            inline void AddExternalEvent(NetEvent ne) {
                common::SpinLock l(&m_slEE);
                m_lDriverExternalEvents.push_back(ne);
            }

            inline std::list<NetEvent> GetExternalEvents() {
                common::SpinLock l(&m_slEE);
                std::list<NetEvent> tmp;
                tmp.swap(m_lDriverExternalEvents);

                return std::move(tmp);
            }

            void Wakeup();

        private:
            std::vector<NetEvent>            m_vDriverInternalEvents;
            IEventDriver                    *m_pEventDriver;
            common::spin_lock_t              m_slDriver = UNLOCKED;

            common::spin_lock_t              m_slEE = UNLOCKED;
            std::list<NetEvent>              m_lDriverExternalEvents;
            int                              m_notifySendFd;
            int                              m_notifyRecvFd;
            AFileEventHandler               *m_pLocalReadEventHandler;
            std::mutex                       m_mtxPendingDeleteEHLock;
            std::set<AFileEventHandler*>     m_pendingDeleteEventHandlers;
        };
    }  // namespace net
}  // namespace netty

#endif //NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
