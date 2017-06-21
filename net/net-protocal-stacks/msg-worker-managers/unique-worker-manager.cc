/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "unique-worker-manager.h"

namespace netty {
    namespace net {
        AFileEventHandler *UniqueWorkerManager::GetWorkerEventHandler(net_peer_info_t npt) {
            common::SpinLock l(&m_sl);
            return nullptr;
        }

        bool UniqueWorkerManager::PutWorkerEventHandler(AFileEventHandler *workerEventHandler) {
            common::SpinLock l(&m_sl);
        }

        AFileEventHandler* UniqueWorkerManager::RemoveWorkerEventHandler(net_peer_info_t npt) {
            common::SpinLock l(&m_sl);
        }

        AFileEventHandler *UniqueWorkerManager::lookup_worker(net_peer_info_t &npt) {
            auto p = m_hmap_workers.find(npt);
            if (m_hmap_workers.end() != p) {
                return p->second;
            }

            return nullptr;
        }
    }  // namespace net
}  // namespace netty
