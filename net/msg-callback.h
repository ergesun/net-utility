/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_MSG_CALLBACK_H
#define NET_CORE_MSG_CALLBACK_H

#include <functional>
#include <memory>

namespace netty {
    namespace net {
        class RcvMessageRef;
        /**
         * user不需要负责RcvMessage的释放。
         */
        typedef std::function<void(std::shared_ptr<RcvMessageRef>)> MsgCallbackHandler;
    } // namespace net
} // namespace netty


#endif //NET_CORE_MSG_CALLBACK_H
