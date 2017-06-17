/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IEVENTDRIVER_H
#define NET_CORE_IEVENTDRIVER_H

#include <vector>
#include <cstdint>
#include "../../ievent-handler.h"
#include "../network-api/abstract-socket-event-handler.h"

#define EVENT_NONE       0
#define EVENT_READ       1
#define EVENT_WRITE      2

namespace netty {
    namespace net {
        struct NetEvent {
            ASocketEventHandler *eh;
            int                 mask;
        };

        /**
         * 事件驱动器。
         * EventDriver is a wrap of event mechanisms depends on different OS.
         * For example, Linux will use epoll(2), BSD will use kqueue(2) and select will
         * be used for worst condition.
         */
        class IEventDriver {
        public:
            virtual ~IEventDriver() {}

            /**
             *
             * @param nevent
             * @return 0成功
             */
            virtual int32_t Init(int32_t nevent) = 0;

            /**
             *
             * @param socketEventHandler
             * @param cur_mask
             * @param mask
             * @return 0成功
             */
            virtual int32_t AddEvent(ASocketEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) = 0;

            /**
             *
             * @param socketEventHandler
             * @param cur_mask
             * @param del_mask
             * @return 0成功
             */
            virtual int32_t DeleteEvent(ASocketEventHandler *socketEventHandler, int32_t cur_mask, int32_t del_mask) = 0;

            /**
             *
             * @param socketEventHandler
             * @return 0成功
             */
            virtual int32_t DeleteHandler(ASocketEventHandler *socketEventHandler) = 0;

            /**
             *
             * @param events
             * @param tp
             * @return 事件个数
             */
            virtual int32_t EventWait(std::vector<NetEvent> *events, struct timeval *tp) = 0;
        }; // interface IEventDriver
    }  // namespace net
}  // namespace netty
#endif //NET_CORE_IEVENTDRIVER_H
