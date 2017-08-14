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
#include "../../../common/thread-pool.h"
#include "../../../net/net-protocal-stacks/msg-worker-managers/unique-worker-manager.h"

namespace netty {
    namespace test {
        std::mutex              TcpClientTest::s_mtx;
        std::condition_variable TcpClientTest::s_cv;
        void TcpClientTest::Run(std::string &ip) {
            std::shared_ptr<net::net_addr_t> ssp_npt(nullptr);
            common::MemPool memPool;
            std::shared_ptr<net::INetStackWorkerManager> sspMgr = std::shared_ptr<net::INetStackWorkerManager>(new net::UniqueWorkerManager());
            auto netService = net::SocketServiceFactory::CreateService(net::SocketProtocal::Tcp, ssp_npt, &memPool,
                                                                       std::bind(&TcpClientTest::recv_msg,
                                                                                 std::placeholders::_1),
                                                                        sspMgr);
            if (!netService->Start()) {
                throw std::runtime_error("cannot start SocketService");
            }

            net::net_peer_info_t peerInfo = {
                {
                    .addr = ip.c_str(),
                    .port = 2210
                },
                .sp = net::SocketProtocal::Tcp
            };

            for (;;) {
                TestSndMessage *tsm = new TestSndMessage(&memPool, peerInfo, "client request: hello server!");
                bool rc = netService->SendMessage(tsm);
                if (rc) {
                    std::unique_lock<std::mutex> l(s_mtx);
                    s_cv.wait(l);
                } else {
                    usleep(2);
                }
            }

            netService->Disconnect(peerInfo);
            //netService->Stop();
            DELETE_PTR(netService);
        }

        void TcpClientTest::recv_msg(std::shared_ptr<net::NotifyMessage> sspNM) {
            switch (sspNM->GetType()) {
                case net::NotifyMessageType::Message: {
                    net::MessageNotifyMessage *mnm = dynamic_cast<net::MessageNotifyMessage*>(sspNM.get());
                    auto rm = mnm->GetContent();
                    if (rm) {
                        auto respBuf = rm->GetBuffer();
#ifdef WITH_MSG_ID
#ifdef BULK_MSG_ID
                        std::cout << "response = "  << respBuf->Pos << ", " << "message id is { ts = " << rm->GetId().ts
                                  << ", seq = " << rm->GetId().seq << "}" << std::endl;
#else
                        std::cout << "response = "  << respBuf->Pos << ", " << "message id is " << rm->GetId() << "." << std::endl;
#endif
#else
                        std::cout << "response = "  << respBuf->Pos << "." << std::endl;
#endif
                    }
                    break;
                }
                case net::NotifyMessageType::Worker: {
                    net::WorkerNotifyMessage *wnm = dynamic_cast<net::WorkerNotifyMessage*>(sspNM.get());
                    if (wnm) {
                        std::cout << "worker notify message , rc = " << (int)wnm->GetCode() << ", message = " << wnm->What() << std::endl;
                    }
                    break;
                }
                case net::NotifyMessageType::Server: {
                    net::ServerNotifyMessage *snm = dynamic_cast<net::ServerNotifyMessage*>(sspNM.get());
                    if (snm) {
                        std::cout << "server notify message , rc = " << (int)snm->GetCode() << ", message = " << snm->What() << std::endl;
                    }
                    break;
                }
            }

            static common::ThreadPool tp;
            common::ThreadPool::Task t([](void*){
                usleep(1000 * 100);
                s_cv.notify_one();
            });
            tp.AddTask(t);
        }
    }
}
