/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKET_SERVICE_FACTORY_H
#define NET_CORE_SOCKET_SERVICE_FACTORY_H

#include "isocket-service.h"

namespace netty {
    namespace net {
        class SocketServiceFactory {
        public:
            /**
             *
             * @param nlt 如果为空，则是为仅仅一个服务于client的服务，否则为server信息，会开启server的服务。
             * @param cp  worker的管理策略。
             */
            static ISocketService *
            CreateService(std::shared_ptr<net_local_info_t> nlt, INetStackWorkerManager *cp = nullptr);
        }; // class SocketServiceFactory
    } // namespace net
} // namespace netty

#endif //NET_CORE_SOCKET_SERVICE_FACTORY_H
