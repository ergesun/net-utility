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
         * 注：user无需释放。
         * TODO(sunchao)：考虑也做成回收利用？
         */
        class RcvMessage : public Message {
        public:
            RcvMessage(common::MemPoolObject *refMpo, common::MemPool *mp, net_peer_info_t peerInfo,
                       Message::Header h, common::Buffer *buffer) : Message(mp) {
                m_header = h;
                m_pBuffer = buffer;
                m_refMpo = refMpo;
                m_peerInfo = peerInfo;
            }

            ~RcvMessage() {
                m_refMpo->Put();
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

            /**
             * 获取消息内容。
             * 注: user无需释放。
             * @return
             */
            inline const common::Buffer* GetBuffer() const {
                return m_pBuffer;
            }

        private:
            common::Buffer             *m_pBuffer;
            common::MemPoolObject      *m_refMpo;
        };
    } // namespace net
} // namespace netty

#endif //NET_CORE_RECVMESSAGE_H
