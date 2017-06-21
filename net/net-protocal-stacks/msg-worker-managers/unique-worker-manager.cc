/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../nonblocking/socket/network-api/abstract-file-event-handler.h"

#include "unique-worker-manager.h"

namespace netty {
    namespace net {
        AFileEventHandler* UniqueWorkerManager::GetWorkerEventHandler(net_peer_info_t npt) {
            common::SpinLock l(&m_sl);
            return lookup_worker(npt);
        }

        bool UniqueWorkerManager::PutWorkerEventHandler(AFileEventHandler *workerEventHandler) {
            common::SpinLock l(&m_sl);
            auto npt = workerEventHandler->GetSocketDescriptor()->GetPeerInfo();
            auto handler = lookup_worker(npt);
            if (handler) {
                return false;
            } else {
                m_hmap_workers[npt] = workerEventHandler;

                return true;
            }
        }

        AFileEventHandler* UniqueWorkerManager::RemoveWorkerEventHandler(net_peer_info_t npt) {
            common::SpinLock l(&m_sl);
            auto handler = lookup_worker(npt);
            if (handler) {
                m_hmap_workers.erase(npt);
                return handler;
            } else {
                return nullptr;
            }
        }

        inline AFileEventHandler *UniqueWorkerManager::lookup_worker(net_peer_info_t &npt) {
            auto p = m_hmap_workers.find(npt);
            if (m_hmap_workers.end() != p) {
                return p->second;
            }

            return nullptr;
        }
    }  // namespace net
}  // namespace netty
