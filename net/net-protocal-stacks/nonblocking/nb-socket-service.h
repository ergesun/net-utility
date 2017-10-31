/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_NBSOCKETSERVICE_H
#define NET_CORE_NBSOCKETSERVICE_H

#include <memory>
#include <cassert>

#include "../inet-stack-worker-manager.h"
#include "../../abstract-socket-service.h"
#include "socket/event-drivers/ievent-driver.h"
#include "../../message.h"
#include "../../notify-message.h"
#include "nss-config.h"

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
             */
            explicit NBSocketService(NssConfig nssConfig);

            ~NBSocketService() override;

            /**
             * 开启服务。
             * @param m 模式。
             * @return 成功true,失败false.
             */
            bool Start(uint16_t ioThreadsCnt, NonBlockingEventModel m) override;

            /**
             * 一旦stop，就不能再用了(不可以重新start再用)。
             * @return
             */
            bool Stop() override;

            /**
             * 一旦发送成功，则m的所有权便属于了框架，user无需也不可以再管理此SndMessage，m生命周期由框架控制。
             * 如果发送失败，则m的生命周期由调用者控制。
             * @param m
             * @return
             */
            bool SendMessage(SndMessage *m) override;

            /**
             * 断开一个TCP连接。
             * @param peer
             * @return
             */
            bool Disconnect(const net_peer_info_t &peer) override;

        private:
            bool connect(net_peer_info_t &npt);
            void on_stack_connect(AFileEventHandler *handler);
            bool on_logic_connect(AFileEventHandler *handler);
            void on_finish(AFileEventHandler *handler);

        private:
            NssConfig                                 m_conf;
            std::shared_ptr<INetStackWorkerManager>   m_sspMgr;
            AEventManager                            *m_pEventManager = nullptr;
            bool                                      m_bStopped      = true;
        }; // class NBSocketService
    }  // namespace net
} // namespace netty

#endif //NET_CORE_NBSOCKETSERVICE_H
