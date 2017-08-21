/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "nb-socket-service.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"
#include "socket/network-api/posix/event-manager.h"
#include "socket/network-api/posix/tcp/connection-event-handler.h"
#include "../msg-worker-managers/unique-worker-manager.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        NBSocketService::NBSocketService(SocketProtocal sp, std::shared_ptr<net_addr_t> sspNat, uint16_t logicPort,
                                         std::shared_ptr<INetStackWorkerManager> sspMgr,
                                         common::MemPool *memPool, NotifyMessageCallbackHandler msgCallbackHandler) :
            ASocketService(sp, sspNat), m_iLogicPort(logicPort), m_pMemPool(memPool), m_bStopped(false) {
            assert(memPool);
            m_msgCallback = msgCallbackHandler;
            if (sspMgr.get()) {
                m_pNetStackWorkerManager = sspMgr;
            } else {
                m_pNetStackWorkerManager = std::shared_ptr<INetStackWorkerManager>(new UniqueWorkerManager());
            }
        }

        NBSocketService::~NBSocketService() {
            hw_rw_memory_barrier();
            if (!m_bStopped) {
                Stop();
            }

            DELETE_PTR(m_pEventManager);
        }

        bool NBSocketService::Start(uint16_t ioThreadsCnt, NonBlockingEventModel m) {
            m_bStopped = false;
            m_pEventManager = new PosixEventManager(m_sp, m_nlt, m_pMemPool, MAX_EVENTS, ioThreadsCnt,
                                                    std::bind(&NBSocketService::on_stack_connect, this, _1),
                                                    std::bind(&NBSocketService::on_logic_connect, this, _1),
                                                    std::bind(&NBSocketService::on_finish, this, _1),
                                                    m_msgCallback);
            return m_pEventManager->Start(m);
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
                PosixTcpConnectionEventHandler *eventHandler = nullptr;
                // 既为connect，就是TCP
                PosixTcpClientSocket *ptcs = new PosixTcpClientSocket(npt.nat);
                if (!ptcs->Socket()) {
                    goto Label_failed;
                }
                if (!ptcs->Connect(nullptr)) {
                    ptcs->Close();
                    goto Label_failed;
                }
                // m_iLogicPort： self logic port(for logic service id)
                eventHandler = new PosixTcpConnectionEventHandler(ptcs, m_pMemPool, m_msgCallback, m_iLogicPort,
                                                                  std::bind(&NBSocketService::on_logic_connect, this, _1));
                // logic peer info： peer ip:port。
                eventHandler->GetStackMsgWorker()->GetEventHandler()->GetSocketDescriptor()->SetLogicPeerInfo(net_peer_info_t(npt));
                //m_pEventManager->AddEvent(eventHandler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
                if (!eventHandler->Initialize() || !ptcs->SetNonBlocking(true)) {
                    DELETE_PTR(eventHandler);
                    return false;
                }

                m_pEventManager->AddEvent(eventHandler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
                return true;

            Label_failed:
                DELETE_PTR(ptcs);
                return false;
            } else {
                std::stringstream ss;
                ss << "Not support now!" << __FILE__ << ":" << __LINE__;
                throw std::runtime_error(ss.str());
            }
        }

        bool NBSocketService::Disconnect(net_peer_info_t &npt) {
            auto handler = m_pNetStackWorkerManager->RemoveWorkerEventHandler(npt);
            if (handler) {
                auto ew = handler->GetOwnWorker();
                if (LIKELY(ew)) {
                    ew->AddExternalEpDelEvent(handler);
                    ew->Wakeup();
                }

                return true;
            }

            return false;
        }

        bool NBSocketService::SendMessage(SndMessage *m) {
            if (SocketProtocal::Tcp != m->GetPeerInfo().sp) {
                std::stringstream ss;
                ss << "Not support now!" << __FILE__ << ":" << __LINE__;
                throw std::runtime_error(ss.str());
            }

            bool rc = false;
            auto peer = m->GetPeerInfo();
            auto handler = m_pNetStackWorkerManager->GetWorkerEventHandler(peer);
            if (!handler) {
                if (Connect(peer)) {
                    handler = m_pNetStackWorkerManager->GetWorkerEventHandler(peer);
                }
            }

            if (handler) {
                rc = handler->GetStackMsgWorker()->SendMessage(m);
            } else {
                fprintf(stderr, "There is no worker for peer %s:%d\n", peer.nat.addr.c_str(), peer.nat.port);
            }

            return rc;
        }

        void NBSocketService::on_stack_connect(AFileEventHandler *handler) {
            m_pEventManager->AddEvent(handler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
        }

        bool NBSocketService::on_logic_connect(AFileEventHandler *handler) {
            return m_pNetStackWorkerManager->PutWorkerEventHandler(handler);
        }

        void NBSocketService::on_finish(AFileEventHandler *handler) {
            m_pNetStackWorkerManager->RemoveWorkerEventHandler(handler->GetSocketDescriptor()->GetLogicPeerInfo());
            auto ew = handler->GetOwnWorker();
            if (LIKELY(ew)) {
                ew->AddExternalEpDelEvent(handler);
                ew->Wakeup();
            }
        }
    } // namespace net
} // namespace netty
