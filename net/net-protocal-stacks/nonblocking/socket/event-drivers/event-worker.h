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
         * 事件管理器的封装 -- 使用者的直接类，不要使用IEventDriver。
         * -> 需要通过GetInternalEvent和GetExternalEvents两个函数才能获取所有事件。
         */
        class GCC_INTERNAL EventWorker {
        public:
            struct EpollAddEvent {
                EpollAddEvent(AFileEventHandler *h, int32_t curMask, int32_t m)
                    : socketEventHandler(h), cur_mask(curMask), mask(m) {}
                AFileEventHandler *socketEventHandler = nullptr;
                int32_t cur_mask;
                int32_t mask;
            };

        public:
            EventWorker(uint32_t maxEvents, NonBlockingEventModel m);
            ~EventWorker();

            inline std::vector<NetEvent>* GetEventsContainer() {
                return &m_vDriverInternalEvents;
            }

            int32_t GetInternalEvent(std::vector<NetEvent> *events, struct timeval *tp) {
                return m_pEventDriver->EventWait(events, tp);
            }

            void AddExternalRWOpEvent(NetEvent ne) {
                common::SpinLock l(&m_slEERWOp);
                m_lRwOpExtEvents.push_back(ne);
            }

            std::list<NetEvent> GetExternalRWOpEvents() {
                common::SpinLock l(&m_slEERWOp);
                std::list<NetEvent> tmp;
                tmp.swap(m_lRwOpExtEvents);

                return std::move(tmp);
            }

            void AddExternalEpAddEvent(AFileEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) {
                common::SpinLock l(&m_slEEAddEpEv);
                EpollAddEvent eae = {
                    .socketEventHandler = socketEventHandler,
                    .cur_mask = cur_mask,
                    .mask = mask
                };
                m_lAddEpExtEvents.push_back(eae);
            }

            std::list<EventWorker::EpollAddEvent> GetExternalEpAddEvents() {
                common::SpinLock l(&m_slEEAddEpEv);
                std::list<EpollAddEvent> tmp;
                tmp.swap(m_lAddEpExtEvents);

                return std::move(tmp);
            }

            /**
             * 释放关于h的一切。
             * @param h 该方法会释放h
             */
            void AddExternalEpDelEvent(AFileEventHandler* h) {
                common::SpinLock l(&m_slEEDelEpEv);
                m_lDelEpExtEvents.insert(h);
            }

            std::set<AFileEventHandler*> GetExternalEpDelEvents() {
                common::SpinLock l(&m_slEEDelEpEv);
                std::set<AFileEventHandler*> tmp;
                tmp.swap(m_lDelEpExtEvents);

                return std::move(tmp);
            }

            void Wakeup();

        private:
            friend class PosixEventManager;
            int32_t AddEvent(AFileEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) {
                return m_pEventDriver->AddEvent(socketEventHandler, cur_mask, mask);
            }

            int32_t DeleteHandler(AFileEventHandler *socketEventHandler) {
                return m_pEventDriver->DeleteHandler(socketEventHandler);
            }

        private:
            std::vector<NetEvent>            m_vDriverInternalEvents;
            IEventDriver                    *m_pEventDriver;

            common::spin_lock_t              m_slEERWOp = UNLOCKED;
            std::list<NetEvent>              m_lRwOpExtEvents;
            common::spin_lock_t              m_slEEAddEpEv = UNLOCKED;
            std::list<EpollAddEvent>         m_lAddEpExtEvents;
            common::spin_lock_t              m_slEEDelEpEv = UNLOCKED;
            std::set<AFileEventHandler*>     m_lDelEpExtEvents;
            int                              m_notifySendFd;
            int                              m_notifyRecvFd;
            AFileEventHandler               *m_pLocalReadEventHandler;
        };
    }  // namespace net
}  // namespace netty

#endif //NET_CORE_NB_SOCKET_ED_EVENT_WORKER_H
