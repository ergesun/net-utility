/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <endian.h>

#include "../common/common-utils.h"
#include "../common/codec-utils.h"
#include "../common/buffer.h"

#include "message.h"

namespace netty {
    namespace net {
        common::spin_lock_t Message::m_sIdLock = UNLOCKED;
        Message::Id Message::m_sLastId = Id(0, 0);
        common::spin_lock_t Message::m_sFreeBufferLock = UNLOCKED;
        std::list<common::Buffer*> Message::m_sFreeBuffers = std::list<common::Buffer*>();

        Message::Message(common::MemPool *mp, common::uctime_t deadline, CallbackHandler cb) :
            m_pMemPool(mp), m_deadline(deadline) {
            m_cb = cb;
            m_header.id = get_new_id();
        }

        void Message::Encode() {
            auto headerBufferSize = sizeof(Header);
            auto deriveBufferSize = GetDerivePayloadLength();
            auto totalBufferSize = headerBufferSize + deriveBufferSize;

            m_header.magic = MESSAGE_MAGIC_NO;
            m_header.len = deriveBufferSize;

            auto memPoolObject = m_pMemPool->Get(totalBufferSize);
            m_pBuffer = Message::get_buffer(memPoolObject, totalBufferSize);
            Message::encode_header(m_pBuffer, m_header);
            EncodeDerive(m_pBuffer);
        }

        void Message::Decode(common::Buffer *buffer) {

        }

        void Message::encode_header(common::Buffer *b, Header &h) {
            ByteOrderUtils::WriteUInt64(b->Pos, h.magic);
            b->Pos += sizeof(h.magic);
            ByteOrderUtils::WriteUInt64(b->Pos, (uint64_t)(h.id.ts));
            b->Pos += sizeof(uint64_t);
            ByteOrderUtils::WriteUInt32(b->Pos, h.id.seq);
            b->Pos += sizeof(h.id.seq);
            ByteOrderUtils::WriteUInt32(b->Pos, h.len);
            b->Pos += sizeof(h.len);
        }

        void Message::decode_header(common::Buffer *b, Header &h) {

        }

        Message::Id Message::get_new_id() {
            common::SpinLock l(&m_sIdLock);
            ++m_sLastId.seq;
            if (UNLIKELY(m_sLastId.seq == UINT32_MAX)) {
                m_sLastId.seq = 1;
                m_sLastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            if (UNLIKELY(m_sLastId.ts == 0)) {
                // 0说明没有初始化过，那就
                m_sLastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            return Id(m_sLastId.ts, m_sLastId.seq);
        }

        common::Buffer* Message::get_buffer() {
            common::SpinLock l(&m_sFreeBufferLock);
            if (m_sFreeBuffers.empty()) {
                return new common::Buffer();
            } else {
                auto begin = m_sFreeBuffers.begin();
                auto buf = *begin;
                m_sFreeBuffers.erase(begin);

                return buf;
            }
        }

        common::Buffer* Message::get_buffer(uchar *pos, uchar *last, uchar *start, uchar *end,
                                            common::MemPoolObject *mpo) {
            auto buf = Message::get_buffer();
            buf->Refresh(pos, last, start, end, mpo);
            return buf;
        }

        common::Buffer* Message::get_buffer(common::MemPoolObject *mpo, uint32_t totalBufferSize) {
            auto bufferStart = (uchar*)(mpo->Pointer());
            auto bufferEnd = bufferStart + totalBufferSize - 1;
            return Message::get_buffer(bufferStart, bufferEnd, bufferStart, bufferEnd, mpo);
        }

        common::Buffer* Message::put_buffer(common::Buffer *buffer) {
            common::SpinLock l(&m_sFreeBufferLock);
            buffer->Put();
            m_sFreeBuffers.push_back(buffer);
        }
    } // namespace net
} // namespace netty

