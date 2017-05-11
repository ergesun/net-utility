/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "listen-event-handler.h"

namespace netty {
    namespace net {
        int PosixTcpServerEventHandler::HandleReadEvent() {
            return 0;
        }

        int PosixTcpServerEventHandler::HandleWriteEvent() {
            return 0;
        }
    } // namespace net
} // namespace netty
