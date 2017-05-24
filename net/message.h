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
#include "common-def.h"

#define MESSAGE_MAGIC_NO 0xdeadbeefbeefdead

namespace netty {
    namespace common {
        class Buffer;
    }

    namespace net {
        /**
         * Derive Message的内存空间你可以选择通过内存池mp分配(这样做性能友好)，然后通过placement new来构建你的Message具体类对象。
         * 你需要留下Message的Id，它用来唯一标识一个message，你可能需要用它来分发Message。
         */
        class Message : public ICodec {
        public:
            struct Id {
                __time_t ts;  /* 时间戳，为了就是id回环了之后防重。 */
                uint32_t seq; /* 消息的唯一标识 */

                Id() : ts(0), seq(0) {}
                Id(__time_t t, uint32_t i) : ts(t), seq(i) {}
                Id(const Id &) = default;
                Id& operator=(const Id &) = default;
            };

            struct Header {
                uint64_t  magic; /* 校验的魔法数 */
                Id        id;    /* 消息头 */
                uint32_t  len;   /* 消息数据部的长度 */

                Header() : magic(0), len(0) {}
                Header(Id i, uint32_t l) : id(i), len(l) {}
            };

        public:
            typedef std::function<void(NettyCode, Message*, void*)> CallbackHandler;

        public:
            /**
             *
             * @param mp 内存池，以供缓冲buffer。
             * @param header 本服务的附加头信息
             * @param deadline 排队等待的截止时间，超过这个时间就出队回调发送错误。
             */
            Message(common::MemPool *mp, common::uctime_t deadline, CallbackHandler cb);
            virtual ~Message() = default;

            /**
             * 获取消息的唯一id。
             * @return
             */
            inline Id GetId() {
                return m_header.id;
            }

            /**
             * 获取消息的回调。
             * @return
             */
            inline CallbackHandler GetCallbackHandler() {
                return m_cb;
            }

            /**
             * 可能你需要根据这个来设计自己的payload的大小以对齐或如何，当然很可能你不在意，那就无需关注这个功能。
             * @return
             */
            static uint32_t HeaderSize() {
                return sizeof(Header);
            }

        public: // ICodec interfaces
            /**
             * 发送Message之前必须调用Encode方法对Message的Header以及payload(派生类的数据)进行编码(就是个字节序转换)。
             */
            void Encode() override final;
            /**
             *
             * @param buffer
             */
            void Decode(common::Buffer *buffer) override final;

        protected:
            /**
             * 获取派生类的消息encode之后待发送字节的长度。
             * @return
             */
            virtual uint32_t GetDerivePayloadLength() = 0;
            virtual void EncodeDerive(common::Buffer *b) = 0;

        private:
            static void encode_header(common::Buffer *b, Header &h);
            static void decode_header(common::Buffer *b, Header &h);
            static Id get_new_id();
            static common::Buffer* get_buffer();
            static common::Buffer* get_buffer(uchar *pos, uchar *last, uchar *start, uchar *end, common::MemPoolObject *mpo);
            static common::Buffer* get_buffer(common::MemPoolObject *mpo, uint32_t totalBufferSize);
            static common::Buffer* put_buffer(common::Buffer *buffer);

        private:
            Header              m_header;
            common::uctime_t    m_deadline;
            common::MemPool    *m_pMemPool;
            CallbackHandler     m_cb;
            common::Buffer     *m_pBuffer;

            static common::spin_lock_t m_sIdLock;
            static Id m_sLastId;
            static common::spin_lock_t m_sFreeBufferLock;
            // TODO(sunchao): 做一个个数限制？
            static std::list<common::Buffer*> m_sFreeBuffers;
        }; // interface Message
    } // namespace net
} // namespace netty

#endif //NET_CORE_IMESSAGE_H
