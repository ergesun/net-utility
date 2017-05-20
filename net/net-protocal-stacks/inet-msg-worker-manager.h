/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_ICONNECTIONMANAGER_H
#define NET_CORE_ICONNECTIONMANAGER_H

#include "../../common/common-def.h"
#include "../common-def.h"

namespace netty {
    namespace net {
        class INetMessageWorker;

        /**
         * worker管理器。
         */
        class GCC_INTERNAL INetStackWorkerManager {
        public:
            virtual ~INetStackWorkerManager() = default;

            virtual INetMessageWorker *GetWorker(net_peer_info_t &npt) = 0;

            virtual void PutWorker(INetMessageWorker *conn) = 0;
        }; // interface INetStackWorkerManager
    }  // namespace net
} // namespace netty

#endif //NET_CORE_ICONNECTIONMANAGER_H