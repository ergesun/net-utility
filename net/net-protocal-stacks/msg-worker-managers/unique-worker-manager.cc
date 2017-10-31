/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../nonblocking/socket/network-api/abstract-file-event-handler.h"

#include "unique-worker-manager.h"

namespace netty {
    namespace net {
        UniqueWorkerManager::~UniqueWorkerManager() {
            common::SpinLock l(&m_sl);
            for (auto p : m_hmap_workers) {
                DELETE_PTR(p.second);
            }
            m_hmap_workers.clear();
            m_hmap_rp_lp.clear();
            m_hmap_handler_rp.clear();
        }

        AFileEventHandler* UniqueWorkerManager::GetWorkerEventHandler(const net_peer_info_t &logicNpt) {
            common::SpinLock l(&m_sl);
            return lookup_worker(logicNpt);
        }

        AFileEventHandler* UniqueWorkerManager::GetWorkerEventHandlerWithRef(const net_peer_info_t &logicNpt) {
            common::SpinLock l(&m_sl);
            auto handler = lookup_worker(logicNpt);
            if (handler) {
                handler->AddRef();
            }

            return handler;
        }

        bool UniqueWorkerManager::PutWorkerEventHandler(AFileEventHandler *workerEventHandler) {
            common::SpinLock l(&m_sl);
            auto lnpt = workerEventHandler->GetSocketDescriptor()->GetLogicPeerInfo();
            auto rnpt = workerEventHandler->GetSocketDescriptor()->GetRealPeerInfo();
            auto handler = lookup_worker(lnpt);
            bool res;
            if (handler) {
                res = false;
            } else {
                m_hmap_workers[lnpt] = workerEventHandler;
                m_hmap_rp_lp[rnpt] = lnpt;
                m_hmap_handler_rp[uintptr_t(workerEventHandler)] = rnpt;
                res = true;
            }

            return res;
        }

        AFileEventHandler* UniqueWorkerManager::RemoveWorkerEventHandler(const net_peer_info_t &logicNpt, const net_peer_info_t &realNpt) {
            common::SpinLock l(&m_sl);
            if (m_hmap_rp_lp.find(realNpt) == m_hmap_rp_lp.end()) {
                return nullptr;
            }

            m_hmap_rp_lp.erase(realNpt);
            auto handler = lookup_worker(logicNpt);
            if (handler) {
                m_hmap_workers.erase(logicNpt);
                return handler;
            } else {
                return nullptr;
            }
        }

        AFileEventHandler* UniqueWorkerManager::RemoveWorkerEventHandler(const net_peer_info_t &logicNpt) {
            common::SpinLock l(&m_sl);
            auto handler = lookup_worker(logicNpt);
            if (handler) {
                m_hmap_workers.erase(logicNpt);
                auto realNpt = m_hmap_handler_rp[uintptr_t(handler)];
                m_hmap_handler_rp.erase(uintptr_t(handler));
                m_hmap_rp_lp.erase(realNpt);

                return handler;
            } else {
                return nullptr;
            }
        }

        inline AFileEventHandler *UniqueWorkerManager::lookup_worker(const net_peer_info_t &logicNpt) {
            auto p = m_hmap_workers.find(logicNpt);
            if (m_hmap_workers.end() != p) {
                return p->second;
            }

            return nullptr;
        }
    }  // namespace net
}  // namespace netty
