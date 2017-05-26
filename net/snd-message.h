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
            typedef std::function<void(NettyMsgCode, Message*, void*)> CallbackHandler;

        public:
            /**
             *
             * @param mp
             * @param deadline 排队等待的截止时间，超过这个时间就出队回调发送错误。
             * @param cb 回复消息回调函数
             */
            SndMessage(common::MemPool *mp, common::uctime_t deadline, CallbackHandler cb) :
                Message(mp) {
                m_deadline = deadline;
                m_cb = cb;
            }

        public:
            common::Buffer* Encode() override final;
            /**
             * 获取消息的回调。
             * @return
             */
            inline CallbackHandler GetCallbackHandler() const {
                return m_cb;
            }

            /**
             * 获取死亡时间
             * @return
             */
            inline common::uctime_t GetDeadline() const {
                return m_deadline;
            }

        protected:
            /**
             * 获取派生类的消息encode之后待发送字节的长度。
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

        private:
            CallbackHandler     m_cb;
            common::uctime_t    m_deadline;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_SND_MESSAGE_H
