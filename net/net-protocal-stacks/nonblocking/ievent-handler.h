/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_IEVENTHANDLER_H
#define NET_CORE_POSIX_TCP_IEVENTHANDLER_H

namespace netty {
    namespace net {
        /**
         * 事件处理器。epoll的ptr管理，有事件了回调。
         */
        class IEventHandler {
        public:
            virtual ~IEventHandler() {}

            virtual bool HandleReadEvent() = 0;

            virtual bool HandleWriteEvent() = 0;
        }; // interface IEventHandler
    }  // namespace net
} // namespace netty

#endif //NET_CORE_POSIX_TCP_IEVENTHANDLER_H
