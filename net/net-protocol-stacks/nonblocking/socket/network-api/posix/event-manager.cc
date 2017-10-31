/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "tcp/server-event-handler.h"

#include "event-manager.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        PosixEventManager::PosixEventManager(SocketProtocol sp, std::shared_ptr<net_addr_t> sspNat, common::MemPool *memPool, uint32_t maxEvents,
                                             uint32_t connWorkersCnt, ConnectHandler stackConnectHandler, ConnectFunc logicConnectHandler,
                                             FinishHandler finishHandler, NotifyMessageCallbackHandler msgCallbackHandler)  :
            AEventManager(memPool, maxEvents), m_sp(sp), m_sspNat(std::move(sspNat)), m_iConnWorkersCnt(connWorkersCnt) {
            m_onStackConnect = std::move(stackConnectHandler);
            m_onLogicConnect = std::move(logicConnectHandler);
            m_onFinish = std::move(finishHandler);
            m_msgCallback = std::move(msgCallbackHandler);
        }

        PosixEventManager::~PosixEventManager() {
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_pServerEventHandler);
        }

        bool PosixEventManager::Start(NonBlockingEventModel m) {
            m_bStopped = false;
            if (SocketProtocol::Tcp == m_sp) {
                if (m_sspNat.get()) {
                    auto ew = new EventWorker(m_iMaxEvents, m);
                    m_pServerEventHandler = new PosixTcpServerEventHandler(ew, m_sspNat.get(), m_onStackConnect,
                                                                           m_onLogicConnect,
                                                                           m_pMemPool, m_msgCallback);
                    m_pListenWorkerEventLoopCtx.second = ew;
                    m_pListenWorkerEventLoopCtx.first = new std::thread(std::bind(&PosixEventManager::worker_loop, this, ew));
                    ew->AddExternalEpAddEvent(m_pServerEventHandler, EVENT_NONE, EVENT_READ);
                    ew->Wakeup();
                }

                m_vConnsWorkerEventLoopCtxs.resize(m_iConnWorkersCnt);
                for (uint32_t i = 0; i < m_iConnWorkersCnt; ++i) {
                    auto ew = new EventWorker(m_iMaxEvents, m);
                    m_vConnsWorkerEventLoopCtxs[i].second = ew;
                    m_vConnsWorkerEventLoopCtxs[i].first = new std::thread(std::bind(&PosixEventManager::worker_loop, this, ew));
                }
            } else {
                std::stringstream ss;
                ss << "Not support now!" << __FILE__ << ":" << __LINE__;
                throw std::runtime_error(ss.str());
            }

            return true;
        }

        bool PosixEventManager::Stop() {
            m_bStopped = true;
            if (m_sspNat.get()) {
                // 释放listen的worker及其相关资源
                auto listenEW = m_pListenWorkerEventLoopCtx.second;
                listenEW->Wakeup();
                m_pListenWorkerEventLoopCtx.first->join();
                DELETE_PTR(m_pServerEventHandler);
                DELETE_PTR(listenEW);
                auto t = m_pListenWorkerEventLoopCtx.first;
                DELETE_PTR(t);
            }

            // 释放conn worker及其相关资源
            for (uint32_t i = 0; i < m_iConnWorkersCnt; ++i) {
                auto ew = m_vConnsWorkerEventLoopCtxs[i].second;
                ew->Wakeup();
                m_vConnsWorkerEventLoopCtxs[i].first->join();
                DELETE_PTR(ew);
                auto t = m_vConnsWorkerEventLoopCtxs[i].first;
                DELETE_PTR(t);
            }

            return true;
        }

        void PosixEventManager::AddEvent(AFileEventHandler *socketEventHandler, int cur_mask, int mask) {
            {
                common::SpinLock l(&m_slSelectEvents);
                /**
                 * 轮询各个非listen的worker。
                 */
                ++m_iCurWorkerIdx;
                m_iCurWorkerIdx %= m_iConnWorkersCnt;
            }

            auto ew = m_vConnsWorkerEventLoopCtxs[m_iCurWorkerIdx].second;
            socketEventHandler->SetOwnWorker(ew);
            ew->AddExternalEpAddEvent(socketEventHandler, cur_mask, mask);
            ew->Wakeup();
        }

        void PosixEventManager::worker_loop(EventWorker *ew) {
            auto events = ew->GetEventsContainer();
            while (!m_bStopped) { // 里面有锁，其具有内存屏障作用会刷新m_bStopped.
                auto nevents = ew->GetInternalEvent(events, nullptr);
                auto pendingDeleteEventHandlers = ew->GetExternalEpDelEvents();

                for (auto de : pendingDeleteEventHandlers) {
                    ew->DeleteHandler(de);
                }

                if (nevents > 0) {
                    for (int i = 0; i < nevents; ++i) {
                        if (pendingDeleteEventHandlers.find((*events)[i].eh) == pendingDeleteEventHandlers.end()) {
                            process_event(&(*events)[i]);
                        }
                    }
                }

                auto externalEvents = ew->GetExternalRWOpEvents();
                for (auto &eev : externalEvents) {
                    if (pendingDeleteEventHandlers.find(eev.eh) == pendingDeleteEventHandlers.end()) {
                        process_event(&eev);
                    }
                }

                for (auto deleteEventHandler : pendingDeleteEventHandlers) {
                    DELETE_PTR(deleteEventHandler);
                }
                pendingDeleteEventHandlers.clear();

                auto addEvs = ew->GetExternalEpAddEvents();
                for (auto addEv : addEvs) {
                    ew->AddEvent(addEv.socketEventHandler, addEv.cur_mask, addEv.mask);
                }
                addEvs.clear();
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
