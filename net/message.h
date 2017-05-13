/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IMESSAGE_H
#define NET_CORE_IMESSAGE_H

#include <cstdint>
#include <time.h>

#include "icodec.h"

#define MESSAGE_MAGIC_NO 0xdeadbeefbeefdead

namespace netty {
    namespace net {
        struct Message {
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

            Header header;

        }; // interface Message
    } // namespace net
} // namespace netty

#endif //NET_CORE_IMESSAGE_H
