/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NETTY_NET_NPS_NB_NSS_CONFIG_H
#define NETTY_NET_NPS_NB_NSS_CONFIG_H

#include "../../notify-message.h"

#include "../inet-stack-worker-manager.h"

namespace netty {
    namespace common {
        class MemPool;
    }

    namespace net {
        /**
         * 无需move构造，无需copy构造。
         */
        struct NssConfig {
            NssConfig(SocketProtocal sp, std::shared_ptr<net_addr_t> sspNat, uint16_t logicPort,
                      NetStackWorkerMgrType mgrType, common::MemPool *memPool,
                      NotifyMessageCallbackHandler msgCallbackHandler, timeval connectTimeout) {
                this->sp                 = sp;
                this->sspNat             = sspNat;
                this->logicPort          = logicPort;
                this->netMgrType         = mgrType;
                this->memPool            = memPool;
                this->msgCallbackHandler = msgCallbackHandler;
                this->connectTimeout     = connectTimeout;
            }

            SocketProtocal                          sp;
            std::shared_ptr<net_addr_t>             sspNat;
            uint16_t                                logicPort;
            NetStackWorkerMgrType                   netMgrType;
            /**
             * 关联关系，无需本类释放。
             */
            common::MemPool                        *memPool;
            NotifyMessageCallbackHandler            msgCallbackHandler;
            timeval                                 connectTimeout;
        };
    }
}

#endif //NETTY_NET_NPS_NB_NSS_CONFIG_H
