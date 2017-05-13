/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IDISPATCHER_H
#define NET_CORE_IDISPATCHER_H

#include "message.h"

namespace netty {
    namespace net {
        /**
         * 消息分发器。
         */
        class IDispatcher {
        public:
            /**
             * 快速的分发消息。
             * IDispatcher的实现者要注意实现这个接口的时候只要直接留下message即可，
             * 不要做其他处理以免占用分发着cpu资源。
             * @param m
             */
            virtual void DispatchQuickly(Message &&m) = 0;

            virtual void Dispatch(Message &&m) = 0;
        }; // interface IDispatcher
    } // namespace net
} // namespace netty

#endif //NET_CORE_IDISPATCHER_H
