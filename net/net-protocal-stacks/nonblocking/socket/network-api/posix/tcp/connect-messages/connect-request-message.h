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
            ConnectRequestMessage(common::MemPool *mp, int16_t logicPort) :
                SndMessage(mp, net_peer_info_s()), m_logicPort(std::move(logicPort)) {}

        protected:
            uint32_t GetDerivePayloadLength() override {
                return sizeof(uint16_t)/*logic port*/;
            }

            void EncodeDerive(common::Buffer *b) override {
                ByteOrderUtils::WriteUInt16(b->Pos, (uint16_t)m_logicPort);
            }

        private:
            int16_t          m_logicPort;
        };
    }
}

#endif //NET_UTILITY_CONNECT_MSG_H
