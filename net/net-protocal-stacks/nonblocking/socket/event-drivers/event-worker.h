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
            EventWorker(uint32_t maxEvents, NonBlockingEventModel m);
            ~EventWorker();

            inline std::vector<NetEvent>* GetEventsContainer() {
                return &m_vDriverInternalEvents;
            }

            int32_t GetInternalEvent(std::vector<NetEvent> *events, struct timeval *tp) {
                return m_pEventDriver->EventWait(events, tp);
            }

            int32_t AddEvent(AFileEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) {
                common::SpinLock l(&m_slDriver);
                return m_pEventDriver->AddEvent(socketEventHandler, cur_mask, mask);
            }

            /**
             * 由于事件处理是一个线程，用户事件调用是用户线程。那么就存在一种可能：事件管理线程检测到pipe bad或者pipe close或者其他，
             * 便会发出on_finish事件，这时候回调就会触发移除这个订阅事件及其相关的内存;而与此同时用户线程可能会发送数据，恰巧还推送了外部事件，
             * 那么就有可能使得事件循环在处理外部时间的时候访问到已经被释放的内存。这里我们通过延迟删除动作，把删除动作放到事件处理线程里执行，
             * 这样外部事件与内部事件就都在一个线程内处理，就可以避免这样的问题了。
             * @param socketEventHandler
             * @return
             */
            int32_t DeleteHandler(AFileEventHandler *socketEventHandler) {
                int32_t res;
                {
                    common::SpinLock l(&m_slDriver);
                    res = m_pEventDriver->DeleteHandler(socketEventHandler);
                }
                {
                    std::unique_lock<std::mutex> l(m_mtxPendingDeleteEHLock);
                    m_pendingDeleteEventHandlers.insert(socketEventHandler);
                }

                return res;
            }

            /**
             * TODO(sunchao): 优化此处删除逻辑的设计。
             * @return
             */
            std::set<AFileEventHandler*> GetPendingDeleteEventHandlers() {
                std::unique_lock<std::mutex> l(m_mtxPendingDeleteEHLock);
                std::set<AFileEventHandler*> tmp;
                m_pendingDeleteEventHandlers.swap(tmp);
                return std::move(tmp);
            }

            void AddExternalEvent(NetEvent ne) {
                common::SpinLock l(&m_slEE);
                m_lDriverExternalEvents.push_back(ne);
            }

            std::list<NetEvent> GetExternalEvents() {
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
