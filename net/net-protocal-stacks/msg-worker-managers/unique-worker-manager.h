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
             * 获取一个worker。
             * @param logicNpt
             * @return 查找到的worker。如果不存在则为nullptr。
             */
            AFileEventHandler *GetWorkerEventHandler(net_peer_info_t logicNpt) override;

            /**
             * 放入一个worker。如果已经存在了会失败。
             * 注意：当前的连接管理策略是两点之间同一个连接，并且保留最早的连接，新的put会失败。
             * @param workerEventHandler
             */
            bool PutWorkerEventHandler(AFileEventHandler *workerEventHandler) override;

            /**
             * 移除一个worker。
             * @param logicNpt
             * @return 被移除的worker。如果不存在则为nullptr。
             */
            AFileEventHandler* RemoveWorkerEventHandler(net_peer_info_t logicNpt, net_peer_info_t realNpt) override;

        private:
            inline AFileEventHandler *lookup_worker(net_peer_info_t &logicNpt);

        private:
            common::spin_lock_t                                              m_sl = UNLOCKED;
            /**
             * logic peer -> handler
             */
            std::unordered_map<net_peer_info_t, AFileEventHandler*>          m_hmap_workers;
            std::unordered_map<net_peer_info_t, net_peer_info_t>             m_hmap_rp_lp;
        }; // class UniqueWorkerManager
    }  // namespace net
}  // namespace netty
#endif //NET_CORE_NETSTACKWORKERPOLICY_H
