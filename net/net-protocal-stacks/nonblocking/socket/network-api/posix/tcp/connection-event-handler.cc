/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "connection-event-handler.h"

namespace netty {
    namespace net {
        PosixTcpConnectionEventHandler::PosixTcpConnectionEventHandler(net_addr_t &peerAddr, int sfd, common::MemPool *memPool) {
            m_pClientSocket = new PosixTcpClientSocket(peerAddr, sfd);
            SetSocketDescriptor(m_pClientSocket);
            m_pNetStackWorker = new PosixTcpNetStackWorker(memPool, m_pClientSocket);
            m_pMemPool = memPool;
        }

        PosixTcpConnectionEventHandler::~PosixTcpConnectionEventHandler() {
            m_pClientSocket->Close();
            DELETE_PTR(m_pClientSocket);
            DELETE_PTR(m_pNetStackWorker);
        }

        bool PosixTcpConnectionEventHandler::HandleReadEvent() {
            return m_pNetStackWorker->Recv();
        }

        bool PosixTcpConnectionEventHandler::HandleWriteEvent() {
            return m_pNetStackWorker->Send();
        }

        ANetStackMessageWorker *PosixTcpConnectionEventHandler::GetStackMsgWorker() {
            return m_pNetStackWorker;
        }
    } // namespace net
} // namespace netty
