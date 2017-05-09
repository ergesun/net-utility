/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NBSOCKETSERVICE_H
#define NET_CORE_NBSOCKETSERVICE_H

#include <memory>

#include "../inet-msg-worker-policy.h"
#include "../../abstract-socket-service.h"
#include "event-drivers/ievent-driver.h"

namespace net {
class NBSocketService : public ASocketService {
public:
    /**
     *
     * @param nlt 如果为空，则是为仅仅一个服务于client的服务，否则为server信息，会开启server的服务。
     * @param cp  worker的管理策略。
     */
    NBSocketService(std::shared_ptr<net_local_info_t> nlt, INetStackWorkerPolicy *cp) : ASocketService(nlt), m_workerPolicy(cp) {}
    /**
     * 开启服务。
     * @return 成功true,失败false.
     */
    virtual bool Start(NonBlockingModel m) override;
    virtual bool Stop() override;
    virtual bool Connect(net_peer_info_t &npt) override;
    virtual bool SendMessage(IMessage *m) override;

private:
    INetStackWorkerPolicy *m_workerPolicy = nullptr;
    // TODO(sunchao): 扩展为多driver均衡处理。
    IEventDriver          *m_eventDriver  = nullptr;
}; // class NBSocketService
}  // namespace net

#endif //NET_CORE_NBSOCKETSERVICE_H
