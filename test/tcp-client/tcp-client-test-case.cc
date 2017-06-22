/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <memory>
#include <iostream>

#include "../net/common-def.h"
#include "../common/mem-pool.h"
#include "../net/socket-service-factory.h"
#include "../net/net-protocal-stacks/inet-stack-worker-manager.h"

#include "tcp-client-test-case.h"

namespace netty {
    namespace test {
        void TcpClientTest::Run() {
            std::shared_ptr<net::net_addr_t> ssp_npt(nullptr);
            common::MemPool memPool;
            auto mpo = memPool.Get(21);
            mpo->Put();
            auto netService = net::SocketServiceFactory::CreateService(net::SocketProtocal::Tcp, ssp_npt, &memPool,
                                                                       std::bind(&TcpClientTest::recv_msg,
                                                                                 std::placeholders::_1));
            netService->Start();
        }

        void TcpClientTest::recv_msg(std::shared_ptr<net::NotifyMessage> sspNM) {

        }
    }
}
