/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <memory>
#include <iostream>

#include "../../../net/common-def.h"
#include "../../../common/mem-pool.h"
#include "../../../net/socket-service-factory.h"
#include "../../../net/net-protocal-stacks/inet-stack-worker-manager.h"
#include "../../../net/rcv-message.h"
#include "../../../common/buffer.h"

#include "../test-snd-message.h"

#include "tcp-client-test-case.h"

namespace netty {
    namespace test {
        std::mutex              TcpClientTest::s_mtx;
        std::condition_variable TcpClientTest::s_cv;
        void TcpClientTest::Run() {
            std::shared_ptr<net::net_addr_t> ssp_npt(nullptr);
            common::MemPool memPool;
            auto mpo = memPool.Get(21);
            mpo->Put();
            auto netService = net::SocketServiceFactory::CreateService(net::SocketProtocal::Tcp, ssp_npt, &memPool,
                                                                       std::bind(&TcpClientTest::recv_msg,
                                                                                 std::placeholders::_1));
            netService->Start();
            net::net_peer_info_t peerInfo = {
                {
                    .addr = "127.0.0.1",
                    .port = 2210
                },
                .sp = net::SocketProtocal::Tcp
            };

            TestSndMessage *tsm = new TestSndMessage(&memPool, peerInfo, "client request: hello server!");
            bool rc = netService->SendMessage(tsm);
            if (rc) {
                std::unique_lock<std::mutex> l(s_mtx);
                s_cv.wait(l);
            }
        }

        void TcpClientTest::recv_msg(std::shared_ptr<net::NotifyMessage> sspNM) {
            switch (sspNM->GetType()) {
                case net::NotifyMessageType::Message: {
                    net::RcvMessage *rm = dynamic_cast<net::RcvMessage*>(sspNM.get());
                    if (rm) {
                        auto respBuf = rm->GetBuffer();
                        std::cout << "response = "  << respBuf->Pos << "." << std::endl;
                    }
                    break;
                }
                case net::NotifyMessageType::Worker : {
                    break;
                }
                case net::NotifyMessageType::Server: {
                    break;
                }
            }
        }
    }
}
