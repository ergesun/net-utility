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
            virtual ~ISocketService() {}

            virtual bool Start(uint16_t ioThreadsCnt = (uint16_t)(common::CPUS_CNT / 2),
                               NonBlockingEventModel m = NonBlockingEventModel::Posix) = 0;

            virtual bool Stop() = 0;

            virtual bool Connect(net_peer_info_t &npt) = 0;

            virtual bool Disconnect(net_peer_info_t &npt) = 0;

            /**
             * 一旦发送，则m的所有权便属于了框架，user无需也不可以再管理此SndMessage，m生命周期由框架控制。
             * @param m
             * @return
             */
            virtual bool SendMessage(SndMessage *m) = 0;
        }; // interface ISocketService
    } // namespace net
} // namespace netty

#endif //NET_CORE_INET_SERVICE_H
