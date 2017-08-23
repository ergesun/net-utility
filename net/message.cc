/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../common/buffer.h"

#include "message.h"

namespace netty {
    namespace net {
        common::spin_lock_t Message::s_freeBufferLock = UNLOCKED;
        std::list<common::Buffer*> Message::s_freeBuffers = std::list<common::Buffer*>();

        Message::Message(common::MemPool *mp) :
            m_pMemPool(mp) {
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

        void Message::PutBuffer(common::Buffer *buffer) {
            common::SpinLock l(&s_freeBufferLock);
            buffer->Put();
            s_freeBuffers.push_back(buffer);
        }
    } // namespace net
} // namespace netty

