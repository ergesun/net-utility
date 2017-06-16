/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NBSOCKETSERVICE_H
#define NET_CORE_NBSOCKETSERVICE_H

#include <memory>

#include "../inet-msg-worker-manager.h"
#include "../../abstract-socket-service.h"
#include "socket/event-drivers/ievent-driver.h"
#include "../../message.h"

// TODO(sunchao): 可配值
#define MAX_EVENTS  256

namespace netty {
    namespace net {
        class NBSocketService : public ASocketService {
        public:
            /**
             *
             * @param nlt 如果为空，则是为仅仅一个服务于client的服务，否则为server信息，会开启server的服务。
             * @param cp  worker的管理策略。
             * @param memPool 内存池。
             */
            NBSocketService(std::shared_ptr<net_local_info_t> nlt, INetStackWorkerManager *cp, common::MemPool *memPool) :
                ASocketService(nlt), m_workerPolicy(cp), m_memPool(memPool), m_bStopped(false) {}

            ~NBSocketService();

            /**
             * 开启服务。
             * @return 成功true,失败false.
             */
            virtual bool Start(NonBlockingEventModel m) override;

            virtual bool Stop() override;

            virtual bool Connect(net_peer_info_t &npt) override;

            virtual bool SendMessage(Message *m) override;

        private:
            void process_events();

        private:
            std::thread            *m_pEventLoopThread = nullptr;
            INetStackWorkerManager *m_workerPolicy = nullptr;
            // TODO(sunchao): 扩展为多driver均衡处理。
            IEventDriver           *m_eventDriver = nullptr;
            // 无需本类释放。
            common::MemPool        *m_memPool = nullptr;
            SocketEventHandler     *m_srvEventHandler = nullptr;

            bool                    m_bStopped;
            std::vector<NetEvent>   m_vEvents;
        }; // class NBSocketService
    }  // namespace net
} // namespace netty

#endif //NET_CORE_NBSOCKETSERVICE_H
