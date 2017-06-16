/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "socket/event-drivers/epoll/epoll-event-driver.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"

namespace netty {
    namespace net {
        NBSocketService::~NBSocketService() {
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_eventDriver);
            DELETE_PTR(m_srvEventHandler);
            DELETE_PTR(m_workerPolicy);
            DELETE_PTR(m_pEventLoopThread);
        }

        bool NBSocketService::Start(NonBlockingEventModel m) {
            if (NonBlockingEventModel::DPDK == m) {
#ifdef HAVE_DPDK
                // m_eventDriver = new DPDKDriver(cct);
#endif
            } else {
#ifdef __linux__ // HAVE_EPOLL
                m_eventDriver = new EpollEventDriver();
#else
#ifdef (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__) // HAVE_KQUEUE
                // m_eventDriver = new KqueueDriver();
#else
                // m_eventDriver = new SelectDriver();
#endif
#endif
            }

            m_vEvents.resize(MAX_EVENTS);
            m_eventDriver->Init(MAX_EVENTS);
            if (m_nlt.get()) {
                if (SocketProtocal::Tcp == m_nlt->sp) {
                    m_srvEventHandler = new PosixTcpServerEventHandler(m_nlt->nat, m_eventDriver);
                    m_eventDriver->AddEvent(m_srvEventHandler, EVENT_NONE, EVENT_READ);
                } else {
                    throw std::runtime_error("Not support now!");
                }
            }

            m_pEventLoopThread = new std::thread(std::bind(&NBSocketService::process_events, this));

            return 0;
        }

        bool NBSocketService::Stop() {
            m_bStopped = true;
            m_pEventLoopThread->join();
            return 0;
        }

        bool NBSocketService::Connect(net_peer_info_t &npt) {

        }

        bool NBSocketService::SendMessage(Message *m) {

        }

        void NBSocketService::process_events() {
            timeval tp = {
                // 两秒一跳的回调以对stop状态进行检测
                .tv_sec = 2,
                .tv_usec = 0
            };

            while (!m_bStopped) {
                auto nevents = m_eventDriver->EventWait(m_vEvents, &tp);
                if (nevents > 0) {
                    for (int i = 0; i < nevents; ++i) {
                        auto evMask = m_vEvents[i].mask;
                        bool rc = true;
                        if (evMask & EVENT_READ) {
                            rc = m_vEvents[i].eh->HandleReadEvent();
                        }

                        if (rc && (evMask & EVENT_WRITE)) {
                            rc = m_vEvents[i].eh->HandleWriteEvent();
                        }

                        if (!rc) {
                            // 失败了就移除事件并且释放socket资源。
                            if (0 == m_eventDriver->DeleteHandler(m_vEvents[i].eh)) {
                                DELETE_PTR(m_vEvents[i].eh);
                            }
                        }
                    }
                }
            }
        }
    } // namespace net
} // namespace netty
