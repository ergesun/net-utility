/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
#define NET_CORE_POSIX_TCP_NET_STACK_WORKER_H

#include "../../../../../../../common/common-def.h"
#include "../../net-stack-msg-worker.h"
#include "stack/connection-socket.h"
#include "../../abstract-event-manager.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        /**
         * Posix tcp的消息处理类。事件管理器有事件了会调用。
         */
        class GCC_INTERNAL PosixTcpNetStackWorker : public ANetStackMessageWorker {
        public:
            enum CreatorType {
                Client = 0,
                Server
            };
        public:
            PosixTcpNetStackWorker(CreatorType ct, AFileEventHandler *eventHandler, common::MemPool *memPool,
                                   PosixTcpClientSocket *socket, NotifyMessageCallbackHandler msgCallbackHandler,
                                   uint16_t logicPort, ConnectFunc logicConnect);
            PosixTcpNetStackWorker(CreatorType ct, AFileEventHandler *eventHandler, common::MemPool *memPool,
                                   PosixTcpClientSocket *socket, NotifyMessageCallbackHandler msgCallbackHandler,
                                   ConnectFunc logicConnect);
            ~PosixTcpNetStackWorker() override;

            bool Initialize() override;
            /**
             * 错误: 返回false(无论是[socket错误或对端关闭]还是[codec校验错误])
             * 正常: 返回true(即便是遇到了EAGAIN，只要没有发生错误)
             * @return
             */
            bool Recv() override;

            /**
             * 发送缓冲队列里面的数据。
             * @return
             */
            bool Send() override;

        private:
            enum class ConnectionState {
                Connecting,
                ConnectSe,    /* client发起connect之后 */
                ConnectingRe, /* server回复之后 */
                WaitLastACK,  /* server connected之后需要回复 */
                Connected
            };

        private:
            void handshake(RcvMessage *rm);

        private:
            static WorkerNotifyMessage* get_closed_by_peer_worker_message(std::string &&msg);
            static WorkerNotifyMessage* get_broken_worker_message(std::string &&msg);

        private:
            /**
             * 创建者类型，用于框架概念内的握手逻辑。
             */
            CreatorType              m_ct;
            /**
             * 此为弱引用关系，关联关系，外部创建者会释放，本类无需释放。
             */
            PosixTcpClientSocket    *m_pSocket;
            NetWorkerState           m_rcvState       = NetWorkerState::StartToRcvHeader;
            Message::Header          m_header;
            common::Buffer          *m_payloadBuffer  = nullptr;
            common::Buffer          *m_pSendingBuffer = nullptr;
            ConnectionState          m_connState      = ConnectionState::Connecting;
            bool                     m_bConnHandShakeCompleted = false;
            std::mutex               m_initWaitMtx;
            std::condition_variable  m_initWaitCv;
            uint16_t                 m_iLogicPort;
            ConnectFunc              m_onLogicConnect;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
