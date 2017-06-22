/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_TEST_TC_TEST_SND_MESSAGE_H
#define NET_TEST_TC_TEST_SND_MESSAGE_H

#include "../../net/snd-message.h"
#include "../../net/common-def.h"

namespace netty {
    namespace test {
        class TestSndMessage : public net::SndMessage {
        public:
            TestSndMessage(common::MemPool *mp, net::net_peer_info_t socketInfo, std::string msg);

        protected:
            uint32_t GetDerivePayloadLength() override;
            void EncodeDerive(common::Buffer *b) override;

        private:
            std::string    m_str;
        };
    }
}

#endif //NET_TEST_TC_TEST_SND_MESSAGE_H
