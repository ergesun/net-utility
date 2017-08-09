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

/**
 * 如果你需要为消息分配一个唯一的id(进程全局的)用于分发等目的，则开启此宏。
 */
//#define WITH_MSG_ID

/**
 * 如果你的消息交互非常非常频繁，频繁到(假如业务判定一个消息发出后10秒收到不应用层ACK就认为失效)每秒发送429496729个，
 * 那么就要开启这个宏以保证消息序号的正确性。当然这块实现可以改为开启后不加ts而是用uint64_t代替以节省流量。
 */
//#define BIG_MSG_ID
#ifdef WITH_MSG_ID
#ifdef BIG_MSG_ID
#define MSG_HEADER_SIZE 20 // 20 = sizeof(magic) + sizeof(id.ts) + sizeof(id.seq) + sizeof(len)
#else
#define MSG_HEADER_SIZE 12 // 12 = sizeof(magic) + sizeof(id) + sizeof(len)
#endif
#else
#define MSG_HEADER_SIZE 8  // 8 = sizeof(magic) + sizeof(len)
#endif

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
#ifdef WITH_MSG_ID
#ifdef BIG_MSG_ID
            struct Id {
                __time_t ts;  /* 时间戳，为了就是id回环了之后防重。 */
                uint32_t seq; /* 消息的唯一标识 */

                Id() : ts(0), seq(0) {}
                Id(__time_t t, uint32_t i) : ts(t), seq(i) {}
                Id(const Id &) = default;
                Id& operator=(const Id &) = default;
            };
#else
            typedef uint32_t Id;
#endif
#endif

            /**
             * 消息头。
             */
            struct Header {
                uint32_t  magic; /* 校验的魔法数 */
#ifdef WITH_MSG_ID
                Id        id;    /* 序列号 */
#endif
                uint32_t  len;   /* 数据部的长度 */

                Header() : magic(0), len(0) {}
#ifdef WITH_MSG_ID
                Header(Id i, uint32_t l) : id(i), len(l) {}
#else
                Header(uint32_t l) : len(l) {}
#endif
            };

        public:
            /**
             *
             * @param mp 内存池，以供缓冲buffer。
             */
            Message(common::MemPool *mp);
            virtual ~Message() = default;

#ifdef WITH_MSG_ID
            /**
             * 获取消息的唯一id。
             * @return
             */
            inline Id GetId() const {
                return m_header.id;
            }
#endif

            inline net_peer_info_t GetPeerInfo() const {
                return m_peerInfo;
            }

            /**
             * 可能你需要根据这个来设计自己的payload的大小以对齐或如何，当然很可能你不在意，那就无需关注这个功能。
             * @return
             */
            static uint32_t HeaderSize() {
                return MSG_HEADER_SIZE;
            }

            static common::Buffer* GetNewBuffer();
            static common::Buffer* GetNewBuffer(uchar *pos, uchar *last, uchar *start, uchar *end,
                                                common::MemPoolObject *mpo);
            static common::Buffer* GetNewBuffer(common::MemPoolObject *mpo, uint32_t totalBufferSize);
            static common::Buffer* GetNewAvailableBuffer(common::MemPoolObject *mpo, uint32_t totalBufferSize);
            static void            PutBuffer(common::Buffer *buffer);

        protected:
            Header              m_header;
            common::MemPool    *m_pMemPool;
            net_peer_info_t     m_peerInfo;

        private:
            static common::spin_lock_t              s_freeBufferLock;
            // TODO(sunchao): 做一个个数限制？
            static std::list<common::Buffer*>       s_freeBuffers;
        }; // interface Message
#ifdef WITH_MSG_ID
#ifdef BIG_MSG_ID
        inline bool operator<(const Message::Id &a, const Message::Id &b) {
            return (a.ts < b.ts) || (a.ts == b.ts && a.seq < b.seq);
        }

        inline bool operator==(const Message::Id &a, const Message::Id &b) {
            return a.ts == b.ts && a.seq == b.seq;
        }
#endif
#endif
    } // namespace net
} // namespace netty

#ifdef WITH_MSG_ID
#ifdef BIG_MSG_ID
namespace std {
    template<>
    struct hash<netty::net::Message::Id> {
        size_t operator()(const netty::net::Message::Id &id) const {
            return (size_t)id.seq;
        }
    };
}
#endif
#endif

#endif //NET_CORE_IMESSAGE_H
