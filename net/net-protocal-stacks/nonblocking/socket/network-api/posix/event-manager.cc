/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "tcp/server-event-handler.h"

#include "event-manager.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        PosixEventManager::PosixEventManager(SocketProtocal sp, std::shared_ptr<net_addr_t> sspNat, common::MemPool *memPool, uint32_t maxEvents,
                                                   uint32_t connWorkersCnt, ConnectHandler connectHandler,
                                                   FinishHandler finishHandler, NotifyMessageCallbackHandler msgCallbackHandler)  :
            AEventManager(memPool, maxEvents), m_sp(sp), m_sspNat(sspNat), m_iConnWorkersCnt(connWorkersCnt) {
            m_onConnect = connectHandler;
            m_onFinish = finishHandler;
            m_msgCallback = msgCallbackHandler;
        }

        PosixEventManager::~PosixEventManager() {
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_pServerEventHandler);
        }

        bool PosixEventManager::Start(NonBlockingEventModel m) {
            m_bStopped = false;
            if (SocketProtocal::Tcp == m_sp) {
                if (m_sspNat.get()) {
                    auto ew = new EventWorker(m_iMaxEvents, m);
                    m_pServerEventHandler = new PosixTcpServerEventHandler(ew, m_sspNat.get(),
                                                                           std::bind(&PosixEventManager::on_connect, this, _1),
                                                                           m_pMemPool, m_msgCallback);
                    ew->AddEvent(m_pServerEventHandler, EVENT_NONE, EVENT_READ);
                    m_pListenWorkerEventLoopCtx.second = ew;
                    m_pListenWorkerEventLoopCtx.first = new std::thread(std::bind(&PosixEventManager::worker_loop, this, ew));
                }

                m_vConnsWorkerEventLoopCtxs.resize(m_iConnWorkersCnt);
                for (int i = 0; i < m_iConnWorkersCnt; ++i) {
                    auto ew = new EventWorker(m_iMaxEvents, m);
                    m_vConnsWorkerEventLoopCtxs[i].second = ew;
                    m_vConnsWorkerEventLoopCtxs[i].first = new std::thread(std::bind(&PosixEventManager::worker_loop, this, ew));
                }
            } else {
                throw std::runtime_error("Not support now!");
            }
        }

        bool PosixEventManager::Stop() {
            m_bStopped = true;
            // 释放listen的worker及其相关资源
            auto listenEW = m_pListenWorkerEventLoopCtx.second;
            DELETE_PTR(m_pServerEventHandler);
            DELETE_PTR(listenEW);
            m_pListenWorkerEventLoopCtx.first->join();
            DELETE_PTR(m_pListenWorkerEventLoopCtx.first);

            // 释放conn worker及其相关资源
            for (int i = 0; i < m_iConnWorkersCnt; ++i) {
                auto ew = m_vConnsWorkerEventLoopCtxs[i].second;
                DELETE_PTR(ew);
                m_vConnsWorkerEventLoopCtxs[i].first->join();
                DELETE_PTR(m_vConnsWorkerEventLoopCtxs[i].first);
            }

            return true;
        }

        int PosixEventManager::AddEvent(AFileEventHandler *socketEventHandler, int cur_mask, int mask) {
            {
                common::SpinLock l(&m_slSelectEvents);
                /**
                 * 轮询各个非listen的worker。
                 */
                m_iCurWorkerIdx = ++m_iCurWorkerIdx % m_iConnWorkersCnt;
            }

            auto ew = m_vConnsWorkerEventLoopCtxs[m_iCurWorkerIdx].second;

            socketEventHandler->SetOwnWorker(ew);
            return ew->AddEvent(socketEventHandler, cur_mask, mask);
        }

        void PosixEventManager::worker_loop(EventWorker *ew) {
            auto events = ew->GetEventsContainer();
            memory_barrier(); // 或者给m_bStopped加个volatile关键字
            while (!m_bStopped) {
                auto nevents = ew->GetInternalEvent(events, nullptr);
                if (nevents > 0) {
                    for (int i = 0; i < nevents; ++i) {
                        process_event(&(*events)[i]);
                    }
                }

                auto pendingDeleteEventHandlers = ew->GetPendingDeleteEventHandlers();
                auto externalEvents = ew->GetExternalEvents();
                for (auto &eev : externalEvents) {
                    if (pendingDeleteEventHandlers.find(eev.eh) == pendingDeleteEventHandlers.end()) {
                        process_event(&eev);
                    }
                }

                for (auto deleteEventHandler : pendingDeleteEventHandlers) {
                    DELETE_PTR(deleteEventHandler);
                }

                pendingDeleteEventHandlers.clear();
            }
        }

        void PosixEventManager::on_connect(AFileEventHandler *handler) {
            if (m_onConnect) {
                m_onConnect(handler);
            }
        }

        inline void PosixEventManager::process_event(NetEvent *netEvent) {
            auto evMask = netEvent->mask;
            bool rc = true;
            if (evMask & EVENT_READ) {
                rc = netEvent->eh->HandleReadEvent();
            }

            if (rc && (evMask & EVENT_WRITE)) {
                rc = netEvent->eh->HandleWriteEvent();
            }

            // 失败了就发送结束回调(连接管理者回收资源等)。
            if (!rc) {
                m_onFinish(netEvent->eh);
            }
        }
    } // namespace net
} // namespace netty
