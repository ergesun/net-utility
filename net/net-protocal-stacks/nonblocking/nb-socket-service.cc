/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"
#include "socket/network-api/posix/tcp/event-manager.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        NBSocketService::~NBSocketService() {
            memory_barrier();
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_netStackWorkerManager);
            DELETE_PTR(m_pEventManager);
        }

        bool NBSocketService::Start(NonBlockingEventModel m) {
            m_bStopped = false;
            if (m_nlt.get()) {
                if (SocketProtocal::Tcp == m_nlt->sp) {
                    m_pEventManager = new PosixTcpEventManager(&m_nlt->nat, m_pMemPool, MAX_EVENTS, (uint32_t)(common::CPUS_CNT / 2),
                                                                std::bind(&NBSocketService::on_connect, this, _1, _2));
                    m_pEventManager->Start(m);
                } else {
                    throw std::runtime_error("Not support now!");
                }
            }

            return 0;
        }

        bool NBSocketService::Stop() {
            m_bStopped = true;
            return m_pEventManager->Stop();
        }

        bool NBSocketService::Connect(net_peer_info_t &npt) {

        }

        bool NBSocketService::Disconnect(net_peer_info_t &npt) {

        }

        bool NBSocketService::SendMessage(SndMessage *m) {

        }

        void NBSocketService::on_connect(net_peer_info_t peer, ASocketEventHandler *handler) {
            m_pEventManager->AddEvent(handler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
            m_netStackWorkerManager->PutWorker(handler->GetStackMsgWorker());
        }
    } // namespace net
} // namespace netty
