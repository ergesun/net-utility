/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SND_MESSAGE_H
#define NET_CORE_SND_MESSAGE_H

#include "message.h"

namespace netty {
    namespace net {
        /**
         * 一旦发送，则SndMessage的所有权便属于了net库，user无需再管理此SndMessage。生命周期由net库控制。
         */
        class SndMessage : public Message, public IEncoder {
        public:
            /**
             * 调用此构造一般用与发送新消息，其内部会自动产生一个全局单调递增的id以便回应时进行区分。
             * 之后你需要调用函数设置mem pool和peer info。
             */
            SndMessage();

            /**
             * 调用此构造一般用与发送新消息，其内部会自动产生一个全局单调递增的id以便回应时进行区分。
             * @param mp
             * @param peerInfo 标识所走的协议以及本地socket信息
             */
            SndMessage(common::MemPool *mp, net_peer_info_t &&peerInfo);

#if WITH_MSG_ID // 如果开启了这个，可能你回复消息的时候需要保持id不变，就需要用到此构造函数。
            /**
             * 调用此构造用于发送指定id的消息。比如说你作为某个request的response时可能
             * 需要保证response的id和request一致以方便业务应用。
             * @param mp
             * @param peerInfo 标识所走的协议以及本地socket信息
             * @param id 作为响应时赋予的接收到的消息的id，以便接受者用其分发。
             */
            SndMessage(common::MemPool *mp, net_peer_info_t &&peerInfo, Id id);
#endif

        public:
            common::Buffer* Encode() override final;
            inline void SetMemPool(common::MemPool *mp) {
                m_pMemPool = mp;
            }

            inline void SetPeerInfo(net_peer_info_t &&peerInfo) {
                m_peerInfo = std::move(peerInfo);
            }

            inline void SetId(Id id) {
                m_header.id = id;
            }

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
#ifdef WITH_MSG_ID
            static Id get_new_id();

        private:
            static common::spin_lock_t              s_idLock;
            static Id                               s_lastId;
#endif
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_SND_MESSAGE_H
