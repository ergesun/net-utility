/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "connection-event-handler.h"

namespace netty {
    namespace net {
        PosixTcpConnectionEventHandler::PosixTcpConnectionEventHandler(net_addr_t &peerAddr, int sfd) {
            m_pClientSocket = new PosixTcpClientSocket(peerAddr, sfd);
            SetSocketDescriptor(m_pClientSocket);
        }

        PosixTcpConnectionEventHandler::~PosixTcpConnectionEventHandler() {
            DELETE_PTR(m_pClientSocket);
        }

        int PosixTcpConnectionEventHandler::HandleReadEvent() {
            return 0;
        }

        int PosixTcpConnectionEventHandler::HandleWriteEvent() {
            return 0;
        }
    } // namespace net
} // namespace netty
