/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IMESSAGE_H
#define NET_CORE_IMESSAGE_H

#include <cstdint>
#include <time.h>

#include "iencoder.h"
#include "../common/mem-pool.h"
#include "../common/spin-lock.h"
#include "common-def.h"

#define MESSAGE_MAGIC_NO 0xdeadbeef
#define MAX_MSG_PAYLOAD_SIZE  0x4000000    // 64MiB

namespace netty {
    namespace common {
        class Buffer;
    }

    namespace net {
        class RcvMessage;

        /**
         * 注意：payload上限为64MiB。
         * Derive Message的内存空间你可以选择通过内存池mp分配(这样做性能友好)，然后通过placement new来构建你的Message具体类对象。
         * 你需要留下Message的Id，它用来唯一标识一个message，你可能需要用它来分发Message。
         */
        class Message {
        public:
            typedef void* CallbackCtx;
            /**
             * user需要负责RcvMessage的释放。
             */
            typedef std::function<void(RcvMessage*, void*)> CallbackHandler;
            typedef std::pair<CallbackHandler, CallbackCtx> Callback;

            struct Id {
                __time_t ts;  /* 时间戳，为了就是id回环了之后防重。 */
                uint32_t seq; /* 消息的唯一标识 */

                Id() : ts(0), seq(0) {}
                Id(__time_t t, uint32_t i) : ts(t), seq(i) {}
                Id(const Id &) = default;
                Id& operator=(const Id &) = default;
            };

            /**
             * 消息头。
             */
            struct Header {
                uint32_t  magic; /* 校验的魔法数 */
                Id        id;    /* 序列号 */
                uint32_t  len;   /* 数据部的长度 */

                Header() : magic(0), len(0) {}
                Header(Id i, uint32_t l) : id(i), len(l) {}
            };

        public:
            /**
             *
             * @param mp 内存池，以供缓冲buffer。
             */
            Message(common::MemPool *mp);
            virtual ~Message() = default;

            /**
             * 获取消息的唯一id。
             * @return
             */
            inline Id GetId() {
                return m_header.id;
            }

            /**
             * 可能你需要根据这个来设计自己的payload的大小以对齐或如何，当然很可能你不在意，那就无需关注这个功能。
             * @return
             */
            static uint32_t HeaderSize() {
                return sizeof(Header);
            }

            static Callback* LookupCallback(Id id);
            static void AddCallback(Id id, Callback);
            static void RemoveCallback(Id id);

        protected:
            static Id get_new_id();
            static common::Buffer* get_new_buffer();
            static common::Buffer* get_new_buffer(uchar *pos, uchar *last, uchar *start, uchar *end,
                                                  common::MemPoolObject *mpo);
            static common::Buffer* get_new_buffer(common::MemPoolObject *mpo, uint32_t totalBufferSize);
            static common::Buffer* put_buffer(common::Buffer *buffer);

        protected:

            Header              m_header;
            common::MemPool    *m_pMemPool;

        private:
            static common::spin_lock_t              s_idLock;
            static Id                               s_lastId;
            static common::spin_lock_t              s_freeBufferLock;
            // TODO(sunchao): 做一个个数限制？
            static std::list<common::Buffer*>       s_freeBuffers;
            static common::spin_lock_t              s_cbLock;
            static std::unordered_map<Id, Callback> s_callbacks;
        }; // interface Message

        inline bool operator<(const Message::Id &a, const Message::Id &b) {
            return (a.ts < b.ts) || (a.ts == b.ts && a.seq < b.seq);
        }

        inline bool operator==(const Message::Id &a, const Message::Id &b) {
            return a.ts == b.ts && a.seq == b.seq;
        }
    } // namespace net
} // namespace netty

namespace std {
    template<>
    struct hash<netty::net::Message::Id> {
        size_t operator()(const netty::net::Message::Id &id) const {
            return (size_t)id.seq;
        }
    };
}

#endif //NET_CORE_IMESSAGE_H
