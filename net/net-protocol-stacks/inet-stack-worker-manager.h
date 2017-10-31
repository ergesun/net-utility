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
        enum class NetStackWorkerMgrType {
            Unique   = 0
        };

        class AFileEventHandler;

        /**
         * worker管理器。
         */
        class INetStackWorkerManager {
        public:
            virtual ~INetStackWorkerManager() = default;

            /**
             * 获取handler但不增加其引用。
             * @param npt
             * @return
             */
            virtual AFileEventHandler *GetWorkerEventHandler(const net_peer_info_t &npt) = 0;
            /**
             * 获取handler添加引用。
             * @param npt
             * @return
             */
            virtual AFileEventHandler *GetWorkerEventHandlerWithRef(const net_peer_info_t &npt) = 0;
            virtual bool PutWorkerEventHandler(AFileEventHandler *handler) = 0;
            /**
             * 检测到对端连接断开了时，需要调用这个API去移除handler。
             * @param logicNpt
             * @param realNpt
             * @return
             */
            virtual AFileEventHandler* RemoveWorkerEventHandler(const net_peer_info_t &logicNpt, const net_peer_info_t &realNpt) = 0;
            /**
             * 本端主动断开时，需要调用这个API去移除handler。
             * @param logicNpt
             * @return
             */
            virtual AFileEventHandler* RemoveWorkerEventHandler(const net_peer_info_t &logicNpt) = 0;
        }; // interface INetStackWorkerManager
    }  // namespace net
} // namespace netty

#endif //NET_CORE_ICONNECTIONMANAGER_H
