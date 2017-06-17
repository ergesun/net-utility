/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "server-event-handler.h"

#include "event-manager.h"

namespace netty {
    namespace net {
        PosixTcpEventManager::~PosixTcpEventManager() {
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_pServerEventHandler);
        }

        bool PosixTcpEventManager::Start(NonBlockingEventModel m) {
            m_bStopped = false;
            if (m_pNat) {
                auto ew = new EventWorker(m_iMaxEvents, m);
                m_pServerEventHandler = new PosixTcpServerEventHandler(m_pNat, ew->GetDriver(), m_pMemPool);
                // 不需要lock，因为正常只有主线程会add/delete一次
                ew->GetDriver()->AddEvent(m_pServerEventHandler, EVENT_NONE, EVENT_READ);
                m_pListenWorkerEventLoopCtx.second = ew;
                m_pListenWorkerEventLoopCtx.first = new std::thread(std::bind(&PosixTcpEventManager::worker_loop, this, ew, true));
            }

            m_vConnsWorkerEventLoopCtxs.resize(m_iConnWorkersCnt);
            for (int i = 0; i < m_iConnWorkersCnt; ++i) {
                auto ew = new EventWorker(m_iMaxEvents, m);
                m_vConnsWorkerEventLoopCtxs[i].second = ew;
                m_vConnsWorkerEventLoopCtxs[i].first = new std::thread(std::bind(&PosixTcpEventManager::worker_loop, this, ew, false));
            }
        }

        bool PosixTcpEventManager::Stop() {
            m_bStopped = true;
            // 释放listen的worker及其相关资源
            auto listenEW = m_pListenWorkerEventLoopCtx.second;
            listenEW->GetDriver()->DeleteHandler(m_pServerEventHandler);
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
        }

        int PosixTcpEventManager::AddEvent(SocketEventHandler *socketEventHandler, int cur_mask, int mask) {
            {
                common::SpinLock l(&m_slSelectEvents);
                /**
                 * 轮询各个非listen的worker。
                 */
                m_iCurWorkerIdx = ++m_iCurWorkerIdx % m_iConnWorkersCnt;
            }

            auto ew = m_vConnsWorkerEventLoopCtxs[m_iCurWorkerIdx].second;

            common::SpinLock l(ew->GetSpinLock());
            return ew->GetDriver()->AddEvent(socketEventHandler, cur_mask, mask);
        }

        void PosixTcpEventManager::worker_loop(EventWorker *ew, bool isServer) {
            static timeval tp = {
                // 两秒一跳的回调以对stop状态进行检测
                .tv_sec = 2,
                .tv_usec = 0
            };

            auto eventDriver = ew->GetDriver();
            auto events = ew->GetEventsContainer();
            memory_barrier(); // 或者给m_bStopped加个volatile关键字
            while (!m_bStopped) {
                auto nevents = eventDriver->EventWait(events, &tp);
                if (nevents > 0) {
                    for (int i = 0; i < nevents; ++i) {
                        auto evMask = (*events)[i].mask;
                        bool rc = true;
                        if (evMask & EVENT_READ) {
                            rc = (*events)[i].eh->HandleReadEvent();
                        }

                        if (rc && (evMask & EVENT_WRITE)) {
                            rc = (*events)[i].eh->HandleWriteEvent();
                        }

                        // 失败了就移除事件并且释放socket资源。 server的handler在stop的时候会释放，这里就没必要释放了。
                        if (!rc && !isServer) {
                            auto delRc = 0;
                            {
                                // 可能多线程在add，需要加lock
                                common::SpinLock l(ew->GetSpinLock());
                                delRc = eventDriver->DeleteHandler((*events)[i].eh);
                            }

                            if (0 == delRc) {
                                DELETE_PTR((*events)[i].eh);
                            }
                        }
                    }
                }
            }
        }
    } // namespace net
} // namespace netty
