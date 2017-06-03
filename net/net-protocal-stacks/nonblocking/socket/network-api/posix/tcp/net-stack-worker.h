/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
#define NET_CORE_POSIX_TCP_NET_STACK_WORKER_H

#include "../../../../../../../common/common-def.h"
#include "../../net-stack-msg-worker.h"

namespace netty {
    namespace net {
        /**
         * Posix tcp的消息处理类。事件管理器有事件了会调用。
         */
        class GCC_INTERNAL PosixTcpNetStackWorker : public ANetStackMessageWorker {
        public:
            /**
             * 错误: 返回false(无论是[socket错误或对端关闭]还是[codec校验错误])
             * 正常: 返回true(即便是遇到了EAGAIN，只要没有发生错误)
             * @return
             */
            bool Recv();

            /**
             * 发送缓冲队列里面的数据。
             * @return
             */
            bool Send();

        private:

        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
