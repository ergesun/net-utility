/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NETSTACKWORKERPOLICY_H
#define NET_CORE_NETSTACKWORKERPOLICY_H

#include <unordered_map>
#include <unordered_set>

#include "../inet-stack-worker-manager.h"
#include "../../../common/spin-lock.h"

namespace netty {
    namespace net {
        /**
         * 两个节点间复用同一连接的管理器。
         */
        class UniqueWorkerManager : public INetStackWorkerManager {
        public:
            ~UniqueWorkerManager();


            /**
             * 获取worker handler但不增加其引用。
             * @param logicNpt
             * @return 查找到的worker。如果不存在则为nullptr。
             */
            AFileEventHandler *GetWorkerEventHandler(const net_peer_info_t &logicNpt) override;

            /**
             * 获取worker handler并增加其一个引用。
             * @param npt
             * @return
             */
            AFileEventHandler *GetWorkerEventHandlerWithRef(const net_peer_info_t &logicNpt) override;

            /**
             * 放入一个worker。如果已经存在了会失败。
             * 注意：当前的连接管理策略是两点之间同一个连接，并且保留最早的连接，新的put会失败。
             * @param workerEventHandler
             */
            bool PutWorkerEventHandler(AFileEventHandler *workerEventHandler) override;

            /**
             * 检测到对端连接断开了时，需要调用这个API去移除handler。
             * @param logicNpt
             * @param realNpt
             * @return
             */
            AFileEventHandler* RemoveWorkerEventHandler(const net_peer_info_t &logicNpt, const net_peer_info_t &realNpt) override;

            /**
             * 本端主动断开时，需要调用这个API去移除handler。
             * @param logicNpt
             * @return
             */
            AFileEventHandler* RemoveWorkerEventHandler(const net_peer_info_t &logicNpt) override;

        private:
            inline AFileEventHandler *lookup_worker(const net_peer_info_t &logicNpt);

        private:
            common::spin_lock_t                                              m_sl = UNLOCKED;
            /**
             * logic peer -> handler
             */
            std::unordered_map<net_peer_info_t, AFileEventHandler*>          m_hmap_workers;
            std::unordered_map<net_peer_info_t, net_peer_info_t>             m_hmap_rp_lp;
            std::unordered_map<uintptr_t, net_peer_info_t>                   m_hmap_handler_rp;
        }; // class UniqueWorkerManager
    }  // namespace net
}  // namespace netty
#endif //NET_CORE_NETSTACKWORKERPOLICY_H
