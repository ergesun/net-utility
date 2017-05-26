/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../common/buffer.h"
#include "../common/codec-utils.h"

#include "snd-message.h"

namespace netty {
    namespace net {
        common::Buffer* SndMessage::Encode() {
            auto headerBufferSize = sizeof(Header);
            auto deriveBufferSize = GetDerivePayloadLength();
            if (deriveBufferSize > MAX_MSG_PAYLOAD_SIZE) {
                throw std::runtime_error("message payload cannot more than 64MiB");
            }

            auto totalBufferSize = headerBufferSize + deriveBufferSize;

            m_header.magic = MESSAGE_MAGIC_NO;
            m_header.len = deriveBufferSize;

            auto memPoolObject = m_pMemPool->Get(totalBufferSize);
            auto buf = Message::get_new_buffer(memPoolObject, totalBufferSize);
            SndMessage::encode_header(buf, m_header);
            EncodeDerive(buf);

            return buf;
        }

        void SndMessage::encode_header(common::Buffer *b, Header &h) {
            ByteOrderUtils::WriteUInt64(b->Pos, h.magic);
            b->Pos += sizeof(h.magic);
            ByteOrderUtils::WriteUInt64(b->Pos, (uint64_t)(h.id.ts));
            b->Pos += sizeof(uint64_t);
            ByteOrderUtils::WriteUInt32(b->Pos, h.id.seq);
            b->Pos += sizeof(h.id.seq);
            ByteOrderUtils::WriteUInt32(b->Pos, h.len);
            b->Pos += sizeof(h.len);
        }
    } // namespace net
} // namespace netty
