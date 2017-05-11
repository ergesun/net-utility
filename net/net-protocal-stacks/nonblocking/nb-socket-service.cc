/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "event-drivers/epoll/epoll-event-driver.h"

namespace netty {
    namespace net {
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
