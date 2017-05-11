/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIXTCPCONNECTION_H
#define NET_CORE_POSIXTCPCONNECTION_H

#include "../../../../../common-def.h"
#include "ievent-handler.h"

namespace netty {
    namespace net {
        class PosixTcpServerEventHandler : public IEventHandler {
        public:
            virtual int HandleReadEvent() override;

            virtual int HandleWriteEvent() override;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIXTCPCONNECTION_H
