/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>

#include "connection-event-handler.h"

#include "server-event-handler.h"
#include "../../../../../../../common/common-utils.h"
#include "../../../../../../../common/thread-pool.h"

namespace netty {
    namespace net {
        PosixTcpServerEventHandler::PosixTcpServerEventHandler(EventWorker *ew, net_addr_t *nat,
                                                               ConnectHandler stackConnectHandler, ConnectFunc onLogicConnect,
                                                               common::MemPool *memPool,
                                                               NotifyMessageCallbackHandler msgCallbackHandler) {
            // TODO(sunchao): backlog改成可配置？
            m_pSrvSocket = new PosixTcpServerSocket(nat, 512);
            if (!m_pSrvSocket->Socket()) {
                throw std::runtime_error("server socket err!");
            }
            if (0 != common::CommonUtils::SetNonBlocking(m_pSrvSocket->GetFd())) {
                close(m_pSrvSocket->GetFd());
                throw std::runtime_error("set server socket fd non-blocking err!");
            }
            if (!m_pSrvSocket->SetAddrReuse(true)) {
                throw std::runtime_error("server SetAddrReuse err!");
            }
            if (!m_pSrvSocket->Bind()) {
                throw std::runtime_error("bind err!");
            }
            if (!m_pSrvSocket->Listen()) {
                throw std::runtime_error("listen err!");
            }
            SetSocketDescriptor(m_pSrvSocket);
            SetOwnWorker(ew);
            m_onStackConnect = std::move(stackConnectHandler);
            m_onLogicConnect = std::move(onLogicConnect);
            m_pMemPool = memPool;
            m_msgCallbackHandler = std::move(msgCallbackHandler);
            m_tp = new common::ThreadPool(common::CPUS_CNT * 2);
        }

        PosixTcpServerEventHandler::~PosixTcpServerEventHandler() {
            m_pSrvSocket->Close();
            DELETE_PTR(m_pSrvSocket);
            DELETE_PTR(m_tp);
        }

        bool PosixTcpServerEventHandler::Initialize() {
            return true;
        }

        bool PosixTcpServerEventHandler::HandleReadEvent() {
            struct sockaddr_in client_addr;
            socklen_t sock_len = sizeof(struct sockaddr_in);

            for (;;) {
                bzero(&client_addr, sock_len);
                // 如果用accept，那么就需要把得到的fd通过set_nonblocking设置为non-blocking
                //auto conn_fd = m_pSrvSocket->Accept4((struct sockaddr*)&client_addr, &sock_len, SOCK_NONBLOCK);
                auto conn_fd = m_pSrvSocket->Accept((struct sockaddr*)&client_addr, &sock_len);
                if (-1 == conn_fd) {
                    auto err = errno;
                    if (EAGAIN == err) {
                        break;
                    } else if (EINTR == err || ECONNABORTED == err) {
                        continue;
                    } else {
                        auto snm = new ServerNotifyMessage(ServerNotifyMessageCode::Error, "broken server fd, accept err!");
                        handle_message(snm);
                        std::cerr << __func__ << ": accept errno = " << err << ", msg = " << strerror(err) << std::endl;
                        return false;
                    }
                } else {
                    char addrBuf[20];
                    bzero(addrBuf, sizeof(addrBuf));
                    inet_ntop(AF_INET, &client_addr.sin_addr, addrBuf, sizeof(addrBuf));
                    auto port = ntohs(client_addr.sin_port);
                    std::string addrStr(addrBuf);
                    net_addr_t peerAddr(std::move(addrStr), port);
                    // just IPv4 now
                    // 连接失效的时候再释放。
                    PosixTcpConnectionEventHandler *connEventHandler =
                        new PosixTcpConnectionEventHandler(peerAddr, conn_fd, m_pMemPool, m_msgCallbackHandler, m_onLogicConnect);
                    common::ThreadPool::Task t([=](void*){
                        //m_onStackConnect(connEventHandler);
                        if (connEventHandler->Initialize() && connEventHandler->GetSocket()->SetNonBlocking(true)) {
                            m_onStackConnect(connEventHandler);
                        } else {
                            delete connEventHandler;
                            fprintf(stderr, "connection %s:%d initialize failed!\n", peerAddr.addr.c_str(), port);
                            // 本端当前不回收，等待对端关闭/断开(open keep-alive opt)时回收。否则会有二次回收的问题。
                            //m_onFinish(connEventHandler);
                        }
                    });
                    m_tp->AddTask(t);
                }
            }

            return true;
        }

        bool PosixTcpServerEventHandler::HandleWriteEvent() {
            throw std::runtime_error("Not support -- PosixTcpServerEventHandler::HandleWriteEvent!");
        }

        ANetStackMessageWorker *PosixTcpServerEventHandler::GetStackMsgWorker() {
            throw std::runtime_error("Not support -- PosixTcpServerEventHandler::GetStackMsgWorker!");
        }

        inline void PosixTcpServerEventHandler::handle_message(NotifyMessage *nm) {
            if (m_msgCallbackHandler) {
                std::shared_ptr<NotifyMessage> ssp(nm);
                m_msgCallbackHandler(ssp);
            }
        }
    } // namespace net
} // namespace netty
