/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../common/common-utils.h"
#include "../common/codec-utils.h"
#include "../common/buffer.h"

#include "message.h"

namespace netty {
    namespace net {
        common::spin_lock_t Message::m_sIdlock = UNLOCKED;
        Id Message::m_sLastId = Id(0, 0);
        common::spin_lock_t Message::m_sFreeBufferLock = UNLOCKED;
        std::list<common::Buffer*> Message::m_sFreeBuffers = std::list<common::Buffer*>();

        Message::Message(common::MemPool *mp, common::uctime_t deadline) : m_memPool(mp), m_deadline(deadline) {

        }

        Message::~Message() {

        }

        common::Buffer* Message::Encode() {
            auto headerBufferSize = sizeof(Header);
            auto deriveBufferSize = GetDerivePayloadLength();
            auto totalBufferSize = headerBufferSize + deriveBufferSize + 1/* 参考Buffer的特性是最后多一个字节作为空格所用。 */;

            auto memPoolObject = m_memPool->Get(totalBufferSize);
            auto bufferStart = (uchar*)(memPoolObject->Pointer());
            auto bufferEnd = bufferStart + totalBufferSize - 1;
            common::Buffer *buffer = get_buffer(bufferStart, bufferEnd, bufferStart, bufferEnd, memPoolObject);
            encode_header(buffer, m_header);
            EncodeDerive(buffer);
            return buffer;
        }

        void Message::Decode() {

        }

        void Message::encode_header(common::Buffer *b, Header &h) {
            common::LittleEndianCodecUtils::WriteUInt64(b->Pos, h.magic);
            b->Pos += sizeof(h.magic);
            common::LittleEndianCodecUtils::WriteUInt64(b->Pos, (uint64_t)(h.id.ts));

        }

        Message::Id Message::get_new_id() {
            common::SpinLock l(&m_sIdlock);
            ++m_sLastId.id;
            if (UNLIKELY(m_sLastId.id == UINT32_MAX)) {
                m_sLastId.id = 1;
                m_sLastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            if (UNLIKELY(m_sLastId.ts == 0)) {
                // 0说明没有初始化过，那就
                m_sLastId.ts = common::CommonUtils::GetCurrentTime().sec;
            }

            return Id(m_sLastId.ts, m_sLastId.id);
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
            auto buf = get_buffer();
            buf->Refresh(pos, last, start, end, mpo);
            return buf;
        }

        common::Buffer* Message::put_buffer(common::Buffer *buffer) {
            common::SpinLock l(&m_sFreeBufferLock);
            m_sFreeBuffers.push_back(buffer);
        }
    } // namespace net
} // namespace netty

