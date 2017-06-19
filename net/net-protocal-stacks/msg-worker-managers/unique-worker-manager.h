/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NETSTACKWORKERPOLICY_H
#define NET_CORE_NETSTACKWORKERPOLICY_H

#include <unordered_map>

#include "../inet-msg-worker-manager.h"
#include "../../../common/spin-lock.h"

namespace netty {
    namespace net {
        /**
         * 两个节点间复用同一连接的管理器。
         */
        class GCC_INTERNAL UniqueWorkerManager : public INetStackWorkerManager {
        public:
            ~UniqueWorkerManager() {}

            /**
             * 获取一个worker。
             * @param npt
             * @return
             */
            AFileEventHandler *GetWorkerEventHandler(net_peer_info_t &npt) override;

            /**
             * 放入一个worker。
             * @param workerEventHandler
             */
            void PutWorkerEventHandler(net_peer_info_t npt, AFileEventHandler *workerEventHandler) override;

            void ReleaseWorkerEventHandler(AFileEventHandler *workerEventHandler) override;

        private:
            AFileEventHandler *lookup_worker(net_peer_info_t &npt);

        private:
            common::spin_lock_t                                     m_sl = UNLOCKED;
            std::unordered_map<net_peer_info_t, AFileEventHandler*> m_hmap_workers;
        }; // class UniqueWorkerManager
    }  // namespace net
}  // namespace netty
#endif //NET_CORE_NETSTACKWORKERPOLICY_H
