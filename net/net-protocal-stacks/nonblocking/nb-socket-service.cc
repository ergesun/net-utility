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
            DELETE_PTR(m_eventDriver);
            DELETE_PTR(m_srvEventHandler);
            DELETE_PTR(m_workerPolicy);
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

            // TODO(sunchao): 可配值。
            m_eventDriver->init(256);
            if (m_nlt.get()) {
                if (SocketProtocal::Tcp == m_nlt->sp) {
                    m_srvEventHandler = new PosixTcpServerEventHandler(m_nlt->nat, m_eventDriver);
                    m_eventDriver->add_event(m_srvEventHandler, EVENT_NONE, EVENT_READ);
                } else {
                    throw std::runtime_error("Not support now!");
                }
            }

            return 0;
        }

        bool NBSocketService::Stop() {
            return 0;
        }

        bool NBSocketService::Connect(net_peer_info_t &npt) {

        }

        bool NBSocketService::SendMessage(Message *m) {

        }
    } // namespace net
} // namespace netty
