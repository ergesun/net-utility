/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {
        ANetStackMessageWorker::ANetStackMessageWorker(uint32_t maxCacheMessageCnt) {
            m_bqMessages = common::BlockingQueue<Message*>(maxCacheMessageCnt);
        }

        ANetStackMessageWorker::~ANetStackMessageWorker() {
            m_bqMessages.Clear();
        }

        bool ANetStackMessageWorker::SendMessage(Message *m) {
            auto ret = m_bqMessages.TryPush(m);
            if (!ret) {
                return ret;
            }


        }
    } // namespace net
} // namespace netty
