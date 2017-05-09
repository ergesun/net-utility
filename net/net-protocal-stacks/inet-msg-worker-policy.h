/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_ICONNECTIONMANAGER_H
#define NET_CORE_ICONNECTIONMANAGER_H

#include "../../common/common-def.h"
#include "../common-def.h"

namespace net {
class INetMessageWorker;
/**
 * worker管理策略。
 */
class GCC_INTERNAL INetStackWorkerPolicy {
public:
    virtual ~INetStackWorkerPolicy() = default;
    virtual INetMessageWorker* GetWorker(net_peer_info_t &npt) = 0;
    virtual void PutWorker(INetMessageWorker *conn) = 0;
}; // interface INetStackWorkerPolicy
}  // namespace net

#endif //NET_CORE_ICONNECTIONMANAGER_H
