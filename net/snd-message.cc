/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../common/buffer.h"
#include "../common/codec-utils.h"

#include "snd-message.h"
#include "../common/common-utils.h"

namespace netty {
    namespace net {
        common::spin_lock_t SndMessage::s_idLock = UNLOCKED;
        Message::Id SndMessage::s_lastId = Id(0, 0);

        SndMessage::SndMessage(common::MemPool *mp, net_peer_info_t socketInfo) :
            Message(mp), m_socketInfo(socketInfo) {
            m_header.id = get_new_id();
        }

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
            auto buf = Message::GetNewAvailableBuffer(memPoolObject, totalBufferSize);
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

        Message::Id SndMessage::get_new_id() {
            common::SpinLock l(&s_idLock);
            ++s_lastId.seq;
            if (UNLIKELY(s_lastId.seq == UINT32_MAX)) {
                s_lastId.seq = 1;
                s_lastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            if (UNLIKELY(s_lastId.ts == 0)) {
                // 0说明没有初始化过，那就
                s_lastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            return Id(s_lastId.ts, s_lastId.seq);
        }
    } // namespace net
} // namespace netty
