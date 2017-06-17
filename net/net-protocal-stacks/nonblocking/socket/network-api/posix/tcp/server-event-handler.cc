/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include "server-event-handler.h"
#include "connection-event-handler.h"

namespace netty {
    namespace net {
        PosixTcpServerEventHandler::PosixTcpServerEventHandler(net_addr_t *nat, IEventDriver *ed, common::MemPool *memPool) {
            // TODO(sunchao): backlog改成可配置？
            m_pSrvSocket = new PosixTcpServerSocket(nat, 512);
            m_pSrvSocket->Socket();
            m_pSrvSocket->Bind();
            m_pSrvSocket->Listen();
            SetSocketDescriptor(m_pSrvSocket);
            m_pEventDriver = ed;
            m_pMemPool = memPool;
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
                    if (EAGAIN != err && EINTR != err && ECONNABORTED != err) {
                        std::cerr << __func__ << ": accept errno = " << err << ", msg = " << strerror(err) << std::endl;
                        break;
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
                    PosixTcpConnectionEventHandler *connEventHandler = new PosixTcpConnectionEventHandler(peerAddr, conn_fd, m_pMemPool);
                    m_pEventDriver->AddEvent(connEventHandler, EVENT_NONE, EVENT_WRITE | EVENT_READ);
                }
            }

            return 0;
        }

        bool PosixTcpServerEventHandler::HandleWriteEvent() {
            throw std::runtime_error("Not support!");
        }
    } // namespace net
} // namespace netty
