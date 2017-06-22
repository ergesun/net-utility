/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "test-snd-message.h"
#include "../../common/buffer.h"

namespace netty {
    namespace test {
        TestSndMessage::TestSndMessage(common::MemPool *mp, net::net_peer_info_t socketInfo, std::string msg)  :
            net::SndMessage(mp, socketInfo), m_str(msg) {}

        uint32_t TestSndMessage::GetDerivePayloadLength() {
            return m_str.length();
        }

        void TestSndMessage::EncodeDerive(common::Buffer *b) {
            memcpy(b->Pos, m_str.c_str(), m_str.length());
        }
    }
}

