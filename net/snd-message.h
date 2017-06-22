/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SND_MESSAGE_H
#define NET_CORE_SND_MESSAGE_H

#include "message.h"

namespace netty {
    namespace net {
        class SndMessage : public Message, public IEncoder {
        public:
            /**
             * 一旦发送，则SndMessage的所有权便属于了框架，user无需再管理此SndMessage。生命周期由框架控制。
             * @param mp
             * @param peerInfo 标识所走的协议以及本地socket信息
             * @param cb 回复消息回调函数。当前是IO线程同步回调，设计上要求回调函数快速执行、非阻塞。
             * @param cbCtx 回调时带回的ctx
             */
            SndMessage(common::MemPool *mp, net_peer_info_t peerInfo);

        public:
            common::Buffer* Encode() override final;

        protected:
            /**
             * 获取派生类的消息encode之后待发送字节的长度。
             * 负载的最大长度为MAX_MSG_PAYLOAD_SIZE。
             * @return
             */
            virtual uint32_t GetDerivePayloadLength() = 0;

            /**
             * 派生类需要实现的对自己内容的编码。
             * @param b
             */
            virtual void EncodeDerive(common::Buffer *b) = 0;

        private:
            static void encode_header(common::Buffer *b, Header &h);
            static Id get_new_id();

        private:
            static common::spin_lock_t              s_idLock;
            static Id                               s_lastId;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_SND_MESSAGE_H
