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
        void TcpClientTest::Run(std::string &ip, uint16_t port) {
            std::shared_ptr<net::net_addr_t> ssp_npt(nullptr);
            common::MemPool memPool;
            timeval connTimeout = {
                .tv_sec = 0,
                .tv_usec = 100 * 1000
            };

            net::NssConfig nc = {
                .sp = net::SocketProtocal::Tcp,
                .sspNat = ssp_npt,
                .logicPort = port,
                .netMgrType = net::NetStackWorkerMgrType::Unique,
                .memPool = &memPool,
                .msgCallbackHandler = std::bind(recv_msg, std::placeholders::_1),
                .connectTimeout = connTimeout
            };
            auto netService = net::SocketServiceFactory::CreateService(nc);
            if (!netService->Start(2, netty::net::NonBlockingEventModel::Posix)) {
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
                TestSndMessage *tsm = new TestSndMessage(&memPool, net::net_peer_info_t(peerInfo), "client request: hello server!");
                bool rc = netService->SendMessage(tsm);
                if (rc) {
                    std::unique_lock<std::mutex> l(s_mtx);
                    s_cv.wait(l);
                } else {
                    usleep(2);
                }
            }

            netService->Stop();
            DELETE_PTR(netService);
        }

        void TcpClientTest::recv_msg(std::shared_ptr<net::NotifyMessage> sspNM) {
            switch (sspNM->GetType()) {
                case net::NotifyMessageType::Message: {
                    net::MessageNotifyMessage *mnm = dynamic_cast<net::MessageNotifyMessage*>(sspNM.get());
                    auto rm = mnm->GetContent();
                    if (rm) {
                        auto respBuf = rm->GetDataBuffer();
#ifdef WITH_MSG_ID
#ifdef BULK_MSG_ID
                        std::cout << "response = "  << respBuf->GetPos() << ", " << "message id is { ts = " << rm->GetId().ts
                                  << ", seq = " << rm->GetId().seq << "}" << std::endl;
#else
                        std::cout << "response = "  << respBuf->GetPos() << ", " << "message id is " << rm->GetId() << "." << std::endl;
#endif
#else
                        std::cout << "response = "  << respBuf->GetPos() << "." << std::endl;
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
