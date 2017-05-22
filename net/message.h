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

#define MESSAGE_MAGIC_NO 0xdeadbeefbeefdead

namespace netty {
    namespace net {
        class Message : public ICodec {
        public:
            struct Id {
                __time_t ts; /* 时间戳 */
                uint64_t id; /* 消息的唯一标识 */

                Id(__time_t t, uint64_t i) : ts(t), id(i) {}
            };

            struct Header {
                uint64_t  magic = MESSAGE_MAGIC_NO; /* 校验的魔法数 */
                Id        id;    /* 消息头 */
                uint32_t  len;   /* 消息数据部的长度 */

                Header(Id  i, uint32_t l) : id(i), len(l) {}
            };

        public:
            static Message* CreateMessage(common::MemPool *mp, common::uctime_t deadline);

            /**
             * 序列化之后的消息的bytes长度。
             * @return
             */
            virtual uint32_t GetDataLength() = 0;

        private:
            Message(common::MemPool *mp, Header header, common::uctime_t deadline);
            ~Message();

        private:
            Header m_header;
            common::uctime_t m_deadline;
            common::MemPool *m_memPool;
        }; // interface Message
    } // namespace net
} // namespace netty

#endif //NET_CORE_IMESSAGE_H
