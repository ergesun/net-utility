/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IMESSAGE_H
#define NET_CORE_IMESSAGE_H

#include <cstdint>
#include <time.h>

#include "icodec.h"
#include "../common/mem-pool.h"
#include "../common/spin-lock.h"

#define MESSAGE_MAGIC_NO 0xdeadbeefbeefdead

namespace netty {
    namespace common {
        class Buffer;
    }

    namespace net {
        /**
         * 它的内存空间你可以选择通过内存池mp分配(这样做性能友好)，然后通过placement new来构建你的Message具体类对象。
         */
        class Message : public ICodec {
        private:
            struct Id {
                __time_t ts; /* 时间戳，为了就是id回环了之后防重。 */
                uint32_t id; /* 消息的唯一标识 */

                Id(__time_t t, uint32_t i) : ts(t), id(i) {}
            };

            struct Header {
                uint64_t  magic = MESSAGE_MAGIC_NO; /* 校验的魔法数 */
                Id        id;    /* 消息头 */
                uint32_t  len;   /* 消息数据部的长度 */

                Header() = default;
                Header(Id  i, uint32_t l) : id(i), len(l) {}
            };

        public:
            /**
             *
             * @param mp 内存池，以供缓冲buffer。
             * @param header 本服务的附加头信息
             * @param deadline 排队等待的截止时间，超过这个时间就出队回调发送错误。
             */
            Message(common::MemPool *mp, common::uctime_t deadline);
            virtual ~Message();
            /**
             * 获取消息的唯一id。
             * @return
             */
            inline Id GetHeader() {
                return m_header.id;
            }

            /**
             * 可能你需要根据这个来设计自己的payload的大小以对齐或如何，当然很可能你不在意，那就无需关注这个功能。
             * @return
             */
            static uint32_t HeaderSize() {
                return sizeof(Header);
            }

        public: // ICodec interfaces
            common::Buffer* Encode() override final;
            void Decode() override final;

        protected:
            /**
             * 获取派生类的消息encode之后待发送字节的长度。
             * @return
             */
            virtual uint32_t GetDerivePayloadLength() = 0;
            virtual void EncodeDerive(common::Buffer *b) = 0;
            virtual void DecodeDerive(common::Buffer *b) = 0;

        private:
            static void encode_header(common::Buffer *b, Header &h);
            static Id get_new_id();
            static common::Buffer* get_buffer();
            static common::Buffer* get_buffer(uchar *pos, uchar *last, uchar *start, uchar *end, common::MemPoolObject *mpo);
            static common::Buffer* put_buffer(common::Buffer *buffer);

        private:
            Header              m_header;
            common::uctime_t    m_deadline;
            common::MemPool    *m_memPool;

            static common::spin_lock_t m_sIdlock;
            static Id m_sLastId;
            static common::spin_lock_t m_sFreeBufferLock;
            // TODO(sunchao): 做一个个数限制？
            static std::list<common::Buffer*> m_sFreeBuffers;
        }; // interface Message
    } // namespace net
} // namespace netty

#endif //NET_CORE_IMESSAGE_H
