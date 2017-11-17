/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "nb-socket-service.h"
#include "socket/network-api/posix/tcp/server-event-handler.h"
#include "socket/network-api/posix/event-manager.h"
#include "socket/network-api/posix/tcp/connection-event-handler.h"
#include "../msg-worker-managers/unique-worker-manager.h"

using namespace std::placeholders;

namespace netty {
    namespace net {
        NBSocketService::NBSocketService(NssConfig nssConfig) :
            ASocketService(nssConfig.sp, nssConfig.sspNat), m_conf(nssConfig) {
            assert(nssConfig.memPool);
            switch (nssConfig.netMgrType) {
                case NetStackWorkerMgrType::Unique :{
                    m_sspMgr = std::shared_ptr<INetStackWorkerManager>(new UniqueWorkerManager());
                    break;
                }
            }
        }

        NBSocketService::~NBSocketService() {
            Stop();
            DELETE_PTR(m_pEventManager);
        }

        bool NBSocketService::Start(uint16_t ioThreadsCnt, NonBlockingEventModel m) {
            if (!m_bStopped) {
                return true;
            }
            m_bStopped = false;
            hw_rw_memory_barrier();
            m_pEventManager = new PosixEventManager(m_sp, m_nlt, m_conf.memPool, MAX_EVENTS, ioThreadsCnt,
                                                    std::bind(&NBSocketService::on_stack_connect, this, _1),
                                                    std::bind(&NBSocketService::on_logic_connect, this, _1),
                                                    std::bind(&NBSocketService::on_finish, this, _1),
                                                    m_conf.msgCallbackHandler);
            return m_pEventManager->Start(m);
        }

        bool NBSocketService::Stop() {
            if (m_bStopped) {
                return true;
            }
            m_bStopped = true;
            hw_rw_memory_barrier();
            if (!m_pEventManager->Stop()) {
                return false;
            }
            m_sspMgr.reset();
            return true;
        }

        bool NBSocketService::SendMessage(SndMessage *m) {
            if (UNLIKELY(m_bStopped)) {
                return false;
            }
            if (SocketProtocol::Tcp != m->GetPeerInfo().sp) {
                std::stringstream ss;
                ss << "Not support now!" << __FILE__ << ":" << __LINE__;
                throw std::runtime_error(ss.str());
            }

            bool rc = false;
            auto peer = m->GetPeerInfo();
            auto handler = m_sspMgr->GetWorkerEventHandler(peer);
            if (!handler) {
                if (connect(peer)) {
                    handler = m_sspMgr->GetWorkerEventHandler(peer);
                }
            }

            if (handler) {
                rc = handler->GetStackMsgWorker()->SendMessage(m);
            } else {
                fprintf(stderr, "There is no worker for peer %s:%d\n", peer.nat.addr.c_str(), peer.nat.port);
            }

            return rc;
        }

        bool NBSocketService::Disconnect(const net_peer_info_t &peer) {
            if (SocketProtocol::Tcp != peer.sp) {
                std::cerr << "Not support SocketProtocol " << (int32_t)(peer.sp);
                return false;
            }

            if (UNLIKELY(m_bStopped)) {
                return false;
            }

            auto handler = m_sspMgr->RemoveWorkerEventHandler(peer);
            if (!handler) {
                return true;
            }

            handler->GetStackMsgWorker()->ClearMessage();
            handler->Release();

            return true;
        }

        bool NBSocketService::connect(net_peer_info_t &npt) {
            if (m_sspMgr->GetWorkerEventHandler(npt)) {
                return true;
            }

            if (SocketProtocol::Tcp == npt.sp) {
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
                eventHandler = new PosixTcpConnectionEventHandler(ptcs, m_conf.memPool, m_conf.msgCallbackHandler, m_conf.logicPort,
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

        void NBSocketService::on_stack_connect(AFileEventHandler *handler) {
            if (UNLIKELY(m_bStopped)) {
                return;
            }
            m_pEventManager->AddEvent(handler, EVENT_NONE, EVENT_READ|EVENT_WRITE);
        }

        bool NBSocketService::on_logic_connect(AFileEventHandler *handler) {
            if (UNLIKELY(m_bStopped)) {
                return false;
            }
            return m_sspMgr->PutWorkerEventHandler(handler);
        }

        void NBSocketService::on_finish(AFileEventHandler *handler) {
            if (UNLIKELY(m_bStopped)) {
                return;
            }

            // 无需担心并发的时候NBSocketService::Disconnect函数先被调用从而导致handler被释放了本函数下面还在使用。
            // 因为我们目前的对某个worker的处理都是在同一个线程内按序在PosixEventManager::worker_loop中进行的，
            // 假如NBSocketService::Disconnect的后续动作先进行，那么本函数压根不会被调用;假如本函数先调用，那么NBSocketService::Disconnect
            // 的动作一定会排在后面。
            auto lnpt = handler->GetSocketDescriptor()->GetLogicPeerInfo();
            auto rnpt = handler->GetSocketDescriptor()->GetRealPeerInfo();
            if (!m_sspMgr->RemoveWorkerEventHandler(lnpt, rnpt)) {
                return;
            }

            handler->GetStackMsgWorker()->ClearMessage();
            handler->Release();
        }
    } // namespace net
} // namespace netty
