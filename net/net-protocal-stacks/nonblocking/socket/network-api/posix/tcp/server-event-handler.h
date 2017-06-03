/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIXTCPCONNECTION_H
#define NET_CORE_POSIXTCPCONNECTION_H

#include "../../../../../../../common/common-def.h"
#include "../../../../../../common-def.h"
#include "../../socket-event-handler.h"
#include "stack/server-socket.h"

namespace netty {
    namespace net {
        class GCC_INTERNAL PosixTcpServerEventHandler : public SocketEventHandler {
        public:
            PosixTcpServerEventHandler(PosixTcpServerSocket* srvSocket) : SocketEventHandler(srvSocket),
                                                                          m_pSrvSocket(srvSocket) {}

            virtual int HandleReadEvent() override;
            virtual int HandleWriteEvent() override;

        private:
            /**
             * 传入到了父类中，父类会释放掉，所以自己没必要释放了。
             */
            PosixTcpServerSocket *m_pSrvSocket;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIXTCPCONNECTION_H
