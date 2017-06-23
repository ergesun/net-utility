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

namespace netty {
    namespace net {
        PosixTcpServerEventHandler::PosixTcpServerEventHandler(EventWorker *ew, net_addr_t *nat,
                                                               ConnectHandler onConnect, common::MemPool *memPool,
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
            m_onConnect = onConnect;
            m_pMemPool = memPool;
            m_msgCallbackHandler = msgCallbackHandler;
        }

        PosixTcpServerEventHandler::~PosixTcpServerEventHandler() {
            m_pSrvSocket->Close();
            DELETE_PTR(m_pSrvSocket);
        }

        bool PosixTcpServerEventHandler::HandleReadEvent() {
            struct sockaddr_in client_addr;
            socklen_t sock_len = sizeof(struct sockaddr_in);

            for (;;) {
                bzero(&client_addr, sock_len);
                // 如果用accept，那么就需要把得到的fd通过set_nonblocking设置为non-blocking
                auto conn_fd = m_pSrvSocket->Accept4((struct sockaddr*)&client_addr, &sock_len, SOCK_NONBLOCK);
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
                    // just IPv4 now
                    char addrBuf[20];
                    bzero(addrBuf, sizeof(addrBuf));
                    inet_ntop(AF_INET, &client_addr.sin_addr, addrBuf, sizeof(addrBuf));
                    auto port = ntohs(client_addr.sin_port);
                    std::string addrStr(addrBuf);
                    net_addr_t peerAddr(std::move(addrStr), port);
                    // 连接失效的时候再释放。
                    PosixTcpConnectionEventHandler *connEventHandler =
                        new PosixTcpConnectionEventHandler(peerAddr, conn_fd, m_pMemPool, m_msgCallbackHandler);
                    if (m_onConnect) {
                        m_onConnect(connEventHandler);
                    }
                }
            }

            return true;
        }

        bool PosixTcpServerEventHandler::HandleWriteEvent() {
            throw std::runtime_error("Not support!");
        }

        ANetStackMessageWorker *PosixTcpServerEventHandler::GetStackMsgWorker() {
            throw std::runtime_error("Not support!");
        }

        inline void PosixTcpServerEventHandler::handle_message(NotifyMessage *nm) {
            if (m_msgCallbackHandler) {
                std::shared_ptr<NotifyMessage> ssp(nm);
                m_msgCallbackHandler(ssp);
            }
        }
    } // namespace net
} // namespace netty
