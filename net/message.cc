/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <endian.h>

#include "../common/common-utils.h"
#include "../common/buffer.h"

#include "message.h"

namespace netty {
    namespace net {
        common::spin_lock_t Message::s_idLock = UNLOCKED;
        Message::Id Message::s_lastId = Id(0, 0);
        common::spin_lock_t Message::s_freeBufferLock = UNLOCKED;
        std::list<common::Buffer*> Message::s_freeBuffers = std::list<common::Buffer*>();
        common::spin_lock_t Message::s_cbLock = UNLOCKED;
        std::unordered_map<Message::Id, Message::Callback> Message::s_callbacks = std::unordered_map<Message::Id, Message::Callback>();

        Message::Message(common::MemPool *mp) :
            m_pMemPool(mp) {
            m_header.id = get_new_id();
        }

        Message::Id Message::get_new_id() {
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

        common::Buffer* Message::GetNewBuffer() {
            common::SpinLock l(&s_freeBufferLock);
            if (s_freeBuffers.empty()) {
                return new common::Buffer();
            } else {
                auto begin = s_freeBuffers.begin();
                auto buf = *begin;
                s_freeBuffers.erase(begin);

                return buf;
            }
        }

        common::Buffer* Message::GetNewBuffer(uchar *pos, uchar *last, uchar *start, uchar *end,
                                              common::MemPoolObject *mpo) {
            auto buf = Message::GetNewBuffer();
            buf->Refresh(pos, last, start, end, mpo);
            return buf;
        }

        common::Buffer* Message::GetNewBuffer(common::MemPoolObject *mpo, uint32_t totalBufferSize) {
            auto bufferStart = (uchar*)(mpo->Pointer());
            auto bufferEnd = bufferStart + totalBufferSize - 1;
            return Message::GetNewBuffer(nullptr, nullptr, bufferStart, bufferEnd, mpo);
        }

        common::Buffer* Message::GetNewAvailableBuffer(common::MemPoolObject *mpo, uint32_t totalBufferSize) {
            auto bufferStart = (uchar*)(mpo->Pointer());
            auto bufferEnd = bufferStart + totalBufferSize - 1;
            return Message::GetNewBuffer(bufferStart, bufferEnd, bufferStart, bufferEnd, mpo);
        }

        common::Buffer* Message::PutBuffer(common::Buffer *buffer) {
            common::SpinLock l(&s_freeBufferLock);
            buffer->Put();
            s_freeBuffers.push_back(buffer);
        }

        Message::Callback* Message::LookupCallback(Id id) {
            common::SpinLock l(&s_cbLock);
            auto cbIter = s_callbacks.find(id);
            if (LIKELY(cbIter != s_callbacks.end())) {
                return &cbIter->second;
            }

            return nullptr;
        }

        void Message::AddCallback(Id id, Callback cb) {
            common::SpinLock l(&s_cbLock);
            s_callbacks[id] = cb;
        }

        void Message::RemoveCallback(Id id) {
            common::SpinLock l(&s_cbLock);
            s_callbacks.erase(id);
        }
    } // namespace net
} // namespace netty

