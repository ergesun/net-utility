/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../../common/buffer.h"
#include "../../net/message.h"

#include "test-snd-message.h"

namespace netty {
    namespace test {
        TestSndMessage::TestSndMessage(common::MemPool *mp, net::net_peer_info_t socketInfo, std::string msg)  :
            net::SndMessage(mp, socketInfo), m_str(msg) {}

#ifdef WITH_MSG_ID
        TestSndMessage::TestSndMessage(common::MemPool *mp, net::net_peer_info_t socketInfo, net::Message::Id id, std::string msg)  :
            net::SndMessage(mp, socketInfo, id), m_str(msg) {}
#endif

        uint32_t TestSndMessage::GetDerivePayloadLength() {
            return m_str.length();
        }

        void TestSndMessage::EncodeDerive(common::Buffer *b) {
            memcpy(b->Pos, m_str.c_str(), m_str.length());
        }
    }
}

