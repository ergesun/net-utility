/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_INET_SERVICE_H
#define NET_CORE_INET_SERVICE_H

#include "common-def.h"

namespace netty {
    namespace net {
        class SndMessage;
        class ISocketService {
        public:
            virtual ~ISocketService() = default;
            /**
             * 开启服务。
             * @param m 模式。
             * @return 成功true,失败false.
             */
            virtual bool Start(uint16_t ioThreadsCnt, NonBlockingEventModel m) = 0;

            /**
             * 一旦stop，就不能再用了(不可以重新start再用)。
             * @return
             */
            virtual bool Stop() = 0;

            /**
             * 一旦发送成功，则m的所有权便属于了框架，user无需也不可以再管理此SndMessage，m生命周期由框架控制。
             * 如果发送失败，则m的生命周期由调用者控制。
             * @param m
             * @return
             */
            virtual bool SendMessage(SndMessage *m) = 0;
        }; // interface ISocketService
    } // namespace net
} // namespace netty

#endif //NET_CORE_INET_SERVICE_H
