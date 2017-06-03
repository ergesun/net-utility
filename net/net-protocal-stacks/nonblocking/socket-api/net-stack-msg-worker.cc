/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {
        ANetStackMessageWorker::ANetStackMessageWorker(uint32_t maxCacheMessageCnt) {
            m_bqMessages = new common::BlockingQueue<SndMessage*>(maxCacheMessageCnt);
        }

        ANetStackMessageWorker::~ANetStackMessageWorker() {
            m_bqMessages->Clear();
            delete m_bqMessages;
        }

        bool ANetStackMessageWorker::SendMessage(SndMessage *m) {
            auto ret = m_bqMessages->TryPush(m);
            if (!ret) {
                return ret;
            }
        }

        void ANetStackMessageWorker::HandleMessage(RcvMessage *m) {
            auto pCb = RcvMessage::LookupCallback(m->GetId());
            if (pCb) {
                (*pCb)(m);
            } else {

            }
        }
    } // namespace net
} // namespace netty
