/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKET_SERVICE_FACTORY_H
#define NET_CORE_SOCKET_SERVICE_FACTORY_H

#include "isocket-service.h"
#include "notify-message.h"
#include "net-protocal-stacks/nonblocking/nss-config.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        class INetStackWorkerManager;
        class SocketServiceFactory {
        public:
            /**
             *
             */
            static ISocketService* CreateService(NssConfig nssConfig);
        }; // class SocketServiceFactory
    } // namespace net
} // namespace netty

#endif //NET_CORE_SOCKET_SERVICE_FACTORY_H
