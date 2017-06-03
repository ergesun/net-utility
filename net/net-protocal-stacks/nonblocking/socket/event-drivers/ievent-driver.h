/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IEVENTDRIVER_H
#define NET_CORE_IEVENTDRIVER_H

#include <vector>
#include <cstdint>
#include "../../ievent-handler.h"
#include "../network-api/socket-event-handler.h"

#define EVENT_NONE       0
#define EVENT_READ       1
#define EVENT_WRITE      2

namespace netty {
    namespace net {
        struct NetEvent {
            SocketEventHandler *eh;
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

            virtual int32_t init(int32_t nevent) = 0;

            virtual int32_t add_event(SocketEventHandler *socketEventHandler, int32_t cur_mask, int32_t mask) = 0;

            virtual int32_t del_event(SocketEventHandler *socketEventHandler, int32_t cur_mask, int32_t del_mask) = 0;

            virtual int32_t event_wait(std::vector<NetEvent> &events, struct timeval *tp) = 0;
        }; // interface IEventDriver
    }  // namespace net
}  // namespace netty
#endif //NET_CORE_IEVENTDRIVER_H
