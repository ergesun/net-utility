/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKET_SERVICE_FACTORY_H
#define NET_CORE_SOCKET_SERVICE_FACTORY_H

#include "isocket-service.h"
#include "notify-message.h"

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
             * @param sspNat 如果为空，则是为仅仅一个服务于client的服务，否则为server信息，会开启server的服务。
             * @param memPool 内存池对象
             * @param sspMgr  worker的管理策略，传入nullptr则默认创建UniqueWorkerManager。
             */
            static ISocketService* CreateService(SocketProtocal sp, std::shared_ptr<net_addr_t> sspNat, uint16_t logicPort,
                                                 common::MemPool *memPool, NotifyMessageCallbackHandler msgCallbackHandler,
                                                 std::shared_ptr<INetStackWorkerManager> sspMgr);
        }; // class SocketServiceFactory
    } // namespace net
} // namespace netty

#endif //NET_CORE_SOCKET_SERVICE_FACTORY_H
