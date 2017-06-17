/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"
#include "socket/network-api/posix/tcp/event-manager.h"

namespace netty {
    namespace net {
        NBSocketService::~NBSocketService() {
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_workerPolicy);
            DELETE_PTR(m_pEventManager);
        }

        bool NBSocketService::Start(NonBlockingEventModel m) {
            if (m_nlt.get()) {
                if (SocketProtocal::Tcp == m_nlt->sp) {
                    m_pEventManager = new PosixTcpEventManager(&m_nlt->nat, m_pMemPool, MAX_EVENTS, (uint32_t)(common::CPUS_CNT / 2));
                    m_pEventManager->Start(m);
                } else {
                    throw std::runtime_error("Not support now!");
                }
            }

            return 0;
        }

        bool NBSocketService::Stop() {
            m_bStopped = true;
            return 0;
        }

        bool NBSocketService::Connect(net_peer_info_t &npt) {

        }

        bool NBSocketService::Disconnect(net_peer_info_t &npt) {

        }

        bool NBSocketService::SendMessage(Message *m) {

        }
    } // namespace net
} // namespace netty
