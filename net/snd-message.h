/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SND_MESSAGE_H
#define NET_CORE_SND_MESSAGE_H

#include "message.h"

namespace netty {
    namespace net {
        class RcvMessageRef;
        typedef void* MsgCallbackCtx;
        /**
         * user不需要负责RcvMessage的释放。
         */
        typedef std::function<void(std::shared_ptr<RcvMessageRef>, void*)> MsgCallbackHandler;
        typedef std::pair<MsgCallbackHandler, MsgCallbackCtx> MsgCallback;

        class SndMessage : public Message, public IEncoder {
        public:
            /**
             *
             * @param mp
             * @param socketInfo 标识所走的协议以及本地socket信息
             * @param deadline 排队等待的截止时间，超过这个时间就出队回调发送错误。
             * @param cb 回复消息回调函数
             * @param cbCtx 回调时带回的ctx
             * @param canBeRelease Message可不可以被框架释放。
             */
            SndMessage(common::MemPool *mp, net_local_info_t socketInfo, common::uctime_t deadline,
                       MsgCallbackHandler cb, void *cbCtx, bool canBeRelease = true) :
                Message(mp), m_socketInfo(socketInfo) {
                m_deadline = deadline;
                m_cb = cb;
                m_pCallbackCtx = cbCtx;
                m_bCanBeReleased = canBeRelease;
            }

        public:
            common::Buffer* Encode() override final;
            /**
             * 获取消息的回调。
             * @return
             */
            inline MsgCallbackHandler GetCallbackHandler() const {
                return m_cb;
            }

            /**
             * 获取死亡时间
             * @return
             */
            inline common::uctime_t GetDeadline() const {
                return m_deadline;
            }

            inline net_local_info_t GetSocketInfo() {
                return m_socketInfo;
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

        private:
            MsgCallbackHandler     m_cb;
            common::uctime_t       m_deadline;
            void                  *m_pCallbackCtx;
            bool                   m_bCanBeReleased;
            net_local_info_t       m_socketInfo;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_SND_MESSAGE_H
