/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "../test-snd-message.h"

#include "../../../net/socket-service-factory.h"
#include "../../../net/rcv-message.h"
#include "../../../common/buffer.h"

#include "tcp-server-test-case.h"


namespace netty {
    namespace test {
        net::ISocketService     *TcpServerTest::s_ss = nullptr;
        common::MemPool         *TcpServerTest::m_mp = nullptr;

        void TcpServerTest::Run() {
            auto nat = new net::net_addr_t("127.0.0.1", 2210);
            std::shared_ptr<net::net_addr_t> ssp_npt(nat);
            m_mp = new common::MemPool();
            s_ss = net::SocketServiceFactory::CreateService(net::SocketProtocal::Tcp, ssp_npt, m_mp,
                                                                       std::bind(&TcpServerTest::recv_msg,
                                                                                 std::placeholders::_1));
            s_ss->Start();
        }

        void TcpServerTest::recv_msg(std::shared_ptr<net::NotifyMessage> sspNM) {
            switch (sspNM->GetType()) {
                case net::NotifyMessageType::Message: {
                    net::MessageNotifyMessage *mnm = dynamic_cast<net::MessageNotifyMessage*>(sspNM.get());
                    auto rm = mnm->GetContent();
                    if (rm) {
                        auto respBuf = rm->GetBuffer();
                        std::cout << "request = "  << respBuf->Pos << ", " << "message id is { ts = " << rm->GetId().ts
                                  << ", seq = " << rm->GetId().seq << "}" << std::endl;
                        TestSndMessage *tsm = new TestSndMessage(m_mp, rm->GetPeerInfo(),  rm->GetId(), "server response: hello client!");
                        bool rc = s_ss->SendMessage(tsm);
                        if (rc) {
                        }
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
