/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "unique-worker-manager.h"

namespace netty {
    namespace net {

        ANetStackMessageWorker *UniqueWorkerManager::GetWorker(net_peer_info_t &npt) {
            return nullptr;
        }

        void UniqueWorkerManager::PutWorker(ANetStackMessageWorker *worker) {

        }

        ANetStackMessageWorker *UniqueWorkerManager::lookup_conn(net_peer_info_t &npt) {
            auto p = m_hmap_workers.find(npt);
            if (m_hmap_workers.end() != p) {
                return p->second;
            }

            return nullptr;
        }
    }  // namespace net
}  // namespace netty
