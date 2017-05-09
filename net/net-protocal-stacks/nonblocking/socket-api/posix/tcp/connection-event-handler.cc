/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "connection-event-handler.h"

namespace net {
int PosixTcpConnectionEventHandler::HandleReadEvent() {
    return 0;
}

int PosixTcpConnectionEventHandler::HandleWriteEvent() {
    return 0;
}
}
