/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_RECVMESSAGE_H
#define NET_CORE_RECVMESSAGE_H

#include "message.h"

namespace netty {
    namespace net {
        /**
         * TODO(sunchao)：考虑也做成回收利用？
         */
        class RcvMessage : public Message {
        public:
            RcvMessage(common::MemPool *mp, Message::Header h, common::Buffer *buffer, NettyMsgCode rc) : Message(mp) {
                m_header = h;
                m_pBuffer = buffer;
                m_rc = rc;
            }

            ~RcvMessage() {
                Message::PutBuffer(m_pBuffer);
                m_pBuffer = nullptr;
            }

            /**
             *
             * @param buffer
             * @param header 解析的header
             * @return 解析成功失败
             */
            static bool DecodeMsgHeader(common::Buffer *buffer, Header *header);

            inline common::Buffer* GetBuffer() {
                return m_pBuffer;
            }

            /**
             * 获取消息码。
             * @return
             */
            inline NettyMsgCode GetMessageCode() {
                return m_rc;
            }

        private:
            common::Buffer     *m_pBuffer;
            NettyMsgCode        m_rc;
        };

        class RcvMessageRef {
        public:
            RcvMessageRef(RcvMessage* rm, std::function<void(RcvMessage*)> releaseHandle)
                    : m_ref(rm) {
                m_releaseHandle = releaseHandle;
            }
            ~RcvMessageRef() {
                if (m_releaseHandle) {
                    m_releaseHandle(m_ref);
                    m_ref = nullptr;
                }
            }
            inline const RcvMessage* GetContent() const {
                return m_ref;
            }
        private:
            RcvMessage                      *m_ref;
            std::function<void(RcvMessage*)> m_releaseHandle;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_RECVMESSAGE_H
