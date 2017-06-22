/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "net-protocal-stacks/nonblocking/nb-socket-service.h"

#include "socket-service-factory.h"

namespace netty {
    namespace net {
        ISocketService* SocketServiceFactory::CreateService(SocketProtocal sp, std::shared_ptr<net_addr_t> nlt,
                                            common::MemPool *memPool, NotifyMessageCallbackHandler msgCallbackHandler,
                                            INetStackWorkerManager *cp) {
#ifdef __linux__
            return new NBSocketService(sp, nlt, cp, memPool, msgCallbackHandler);
#else // xio、poll、etc.
            return nullptr;
#endif
        }
    } // namespace net
} // namespace netty

