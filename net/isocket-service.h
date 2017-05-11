/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_INET_SERVICE_H
#define NET_CORE_INET_SERVICE_H

#include "common-def.h"
#include "imessage.h"

namespace net {
class ISocketService {
public:
    virtual ~ISocketService() {}
    virtual bool Start(SocketStackModel m) = 0;
    virtual bool Stop() = 0;
    virtual bool Connect(net_peer_info_t &npt) = 0;
    virtual bool SendMessage(IMessage *m) = 0;
};
}

#endif //NET_CORE_INET_SERVICE_H
