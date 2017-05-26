/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_ICODEC_H
#define NET_CORE_ICODEC_H

#include "../common/mem-pool.h"

namespace netty {
    namespace common {
        class Buffer;
    }

    namespace net {
        /**
         * 编码器接口。
         */
        class IEncoder {
        public:
            virtual ~IEncoder() {}
            virtual common::Buffer* Encode() = 0;
        }; // interface IEncoder
    } // namespace net
} // namespace netty

#endif //NET_CORE_ICODEC_H
