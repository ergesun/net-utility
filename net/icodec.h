/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_ICODEC_H
#define NET_CORE_ICODEC_H

namespace netty {
    namespace net {
        /**
         * 编解码器接口。
         */
        class ICodec {
        public:
            virtual ~ICodec() {}
            virtual void Encode() = 0;
            virtual void Decode() = 0;
        }; // interface ICodec
    } // namespace net
} // namespace netty

#endif //NET_CORE_ICODEC_H
