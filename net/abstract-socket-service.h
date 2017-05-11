/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_ABSTRACT_SOCKET_SERVICE_H
#define NET_CORE_ABSTRACT_SOCKET_SERVICE_H

#include "../common/common-def.h"
#include "isocket-service.h"

namespace netty {
    namespace net {
        class GCC_INTERNAL ASocketService : public ISocketService {
        public:
            ASocketService(std::shared_ptr<net_local_info_t> nl) : m_nlt(nl) {}

        protected:
            /**
             * 本地监听的地址协议信息。
             */
            std::shared_ptr<net_local_info_t> m_nlt;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_ABSTRACT_SOCKET_SERVICE_H
