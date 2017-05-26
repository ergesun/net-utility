/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_RECVMESSAGE_H
#define NET_CORE_RECVMESSAGE_H

#include "message.h"

namespace netty {
    namespace net {
        class RcvMessage : public Message {
        public:
            RcvMessage(common::MemPool *mp, Header h) : Message(mp) {
                m_header = h;
            }

            /**
             *
             * @param buffer
             * @param header 解析的header
             * @return 解析成功失败
             */
            bool DecodeMsgHeader(common::Buffer *buffer, Header *header);

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
            NettyMsgCode           m_rc;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_RECVMESSAGE_H
