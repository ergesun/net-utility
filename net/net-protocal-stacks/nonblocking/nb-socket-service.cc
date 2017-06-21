/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"
#include "socket/network-api/posix/tcp/event-manager.h"
#include "socket/network-api/posix/tcp/connection-event-handler.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        NBSocketService::~NBSocketService() {
            memory_barrier();
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_pNetStackWorkerManager);
            DELETE_PTR(m_pEventManager);
        }

        bool NBSocketService::Start(NonBlockingEventModel m) {
            m_bStopped = false;
            if (m_nlt.get()) {
                if (SocketProtocal::Tcp == m_nlt->sp) {
                    m_pEventManager = new PosixTcpEventManager(&m_nlt->nat, m_pMemPool, MAX_EVENTS, (uint32_t)(common::CPUS_CNT / 2),
                                                                std::bind(&NBSocketService::on_connect, this, _1),
                                                               std::bind(&NBSocketService::on_finish, this, _1),
                                                                m_msgCallback);
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
            if (m_pNetStackWorkerManager->GetWorkerEventHandler(npt)) {
                return true;
            }

            if (SocketProtocal::Tcp == npt.sp) {
                // 既为connect，就是TCP
                PosixTcpClientSocket *ptcs = new PosixTcpClientSocket(npt.nat);
                if (!ptcs->Socket()) {
                    goto Label_failed;
                }
                if (!ptcs->Connect(nullptr)) {
                    goto Label_failed;
                }
                auto eventHandler = new PosixTcpConnectionEventHandler(ptcs, m_pMemPool, m_msgCallback);
                if (!m_pNetStackWorkerManager->PutWorkerEventHandler(eventHandler)) {
                    // put失败代表已经有了。
                    DELETE_PTR(eventHandler);
                } else {
                    m_pEventManager->AddEvent(eventHandler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
                }

                return true;

            Label_failed:
                DELETE_PTR(ptcs);
                return false;
            } else {
                throw std::runtime_error("Not support now!");
            }
        }

        bool NBSocketService::Disconnect(net_peer_info_t &npt) {
            auto handler = m_pNetStackWorkerManager->RemoveWorkerEventHandler(npt);
            if (handler) {
                auto ew = handler->GetOwnWorker();
                ew->DeleteHandler(handler);

                return true;
            }

            return false;
        }

        bool NBSocketService::SendMessage(SndMessage *m) {
            bool rc = false;
            auto peer = m->GetSocketInfo();
            auto handler = m_pNetStackWorkerManager->GetWorkerEventHandler(peer);
            if (!handler) {
                if (Connect(peer)) {
                    handler = m_pNetStackWorkerManager->GetWorkerEventHandler(peer);
                }
            }

            if (handler) {
                rc = handler->GetStackMsgWorker()->SendMessage(m);
            }

            return rc;
        }

        void NBSocketService::on_connect(AFileEventHandler *handler) {
            if (m_pNetStackWorkerManager->PutWorkerEventHandler(handler)) {
                m_pEventManager->AddEvent(handler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
            } else {
                DELETE_PTR(handler);
            }
        }

        void NBSocketService::on_finish(AFileEventHandler *handler) {
            auto ew = handler->GetOwnWorker();
            m_pNetStackWorkerManager->RemoveWorkerEventHandler(handler->GetSocketDescriptor()->GetPeerInfo());
            ew->DeleteHandler(handler);
        }
    } // namespace net
} // namespace netty
