/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_UTILITY_CONNECT_MSG_H
#define NET_UTILITY_CONNECT_MSG_H

#include "../../../../../../../snd-message.h"
#include "../../../../../../../../common/codec-utils.h"
#include "../../../../../../../../common/buffer.h"

namespace netty {
    namespace net {
        class ConnectRequestMessage : public SndMessage {
        public:
            ConnectRequestMessage(common::MemPool *mp, net_local_info_t &&logicLocalInfo) :
                SndMessage(mp, net_peer_info_s()), m_logicLocalInfo(std::move(logicLocalInfo)) {}

        protected:
            uint32_t GetDerivePayloadLength() override {
                return sizeof(uint16_t)/*protocol*/ + sizeof(uint16_t)/*port*/ + m_logicLocalInfo.nat.addr.length()/*addr*/;
            }

            void EncodeDerive(common::Buffer *b) override {
                ByteOrderUtils::WriteUInt16(b->Pos, (uint16_t)m_logicLocalInfo.sp);
                b->Pos += sizeof(uint16_t);
                ByteOrderUtils::WriteUInt16(b->Pos, (uint16_t)m_logicLocalInfo.nat.port);
                b->Pos += sizeof(uint16_t);
                memcpy(b->Pos, m_logicLocalInfo.nat.addr.c_str(), m_logicLocalInfo.nat.addr.length());
            }

        private:
            net_local_info_t          m_logicLocalInfo;
        };
    }
}

#endif //NET_UTILITY_CONNECT_MSG_H
