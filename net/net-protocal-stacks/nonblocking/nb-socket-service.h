/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NBSOCKETSERVICE_H
#define NET_CORE_NBSOCKETSERVICE_H

#include <memory>
#include <cassert>

#include "../inet-msg-worker-manager.h"
#include "../../abstract-socket-service.h"
#include "socket/event-drivers/ievent-driver.h"
#include "../../message.h"

// TODO(sunchao): 可配值
#define MAX_EVENTS  256

namespace netty {
    namespace net {
        class AEventManager;
        /**
         * 支持Tcp/Udp(暂未实现)协议的收发server。
         */
        class NBSocketService : public ASocketService {
        public:
            /**
             *
             * @param nlt 如果为空，则是为仅仅一个服务于client的服务，否则为server信息，会开启server的服务。
             * @param cp  worker的管理策略。
             * @param memPool 内存池。
             */
            NBSocketService(std::shared_ptr<net_local_info_t> nlt, INetStackWorkerManager *cp, common::MemPool *memPool) :
                ASocketService(nlt), m_netStackWorkerManager(cp), m_pMemPool(memPool), m_bStopped(false) {
                assert(memPool);
            }

            ~NBSocketService();

            /**
             * 开启服务。
             * @return 成功true,失败false.
             */
            bool Start(NonBlockingEventModel m) override;

            bool Stop() override;

            bool Connect(net_peer_info_t &npt) override;

            bool Disconnect(net_peer_info_t &npt) override;

            bool SendMessage(SndMessage *m) override;

        private:
            void on_connect(net_peer_info_t peer, ASocketEventHandler *handler);

        private:
            INetStackWorkerManager *m_netStackWorkerManager = nullptr;
            // 关联关系，外部传入的，根据谁创建谁销毁原则，本类无需释放。
            common::MemPool        *m_pMemPool = nullptr;
            AEventManager          *m_pEventManager = nullptr;

            bool                    m_bStopped;
        }; // class NBSocketService
    }  // namespace net
} // namespace netty

#endif //NET_CORE_NBSOCKETSERVICE_H
