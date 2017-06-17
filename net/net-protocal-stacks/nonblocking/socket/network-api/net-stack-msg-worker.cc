/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../../../../../common/mem-pool.h"
#include "../../../../../common/buffer.h"

#include "net-stack-msg-worker.h"
#include "../../../../../common/common-utils.h"

namespace netty {
    namespace net {
        ANetStackMessageWorker::ANetStackMessageWorker(common::MemPool *memPool, uint32_t maxCacheMessageCnt) {
            m_pMemPool = memPool;
            m_bqMessages = new common::BlockingQueue<SndMessage*>(maxCacheMessageCnt);
            auto size = RcvMessage::HeaderSize();
            auto headerMemObj = m_pMemPool->Get(size);
            m_pHeaderBuffer = common::CommonUtils::GetNewBuffer(headerMemObj, size);
        }

        ANetStackMessageWorker::~ANetStackMessageWorker() {
            m_bqMessages->Clear();
            DELETE_PTR(m_bqMessages);
            DELETE_PTR(m_pHeaderBuffer);
        }

        bool ANetStackMessageWorker::SendMessage(SndMessage *m) {
            auto ret = m_bqMessages->TryPush(m);
            if (!ret) {
                return ret;
            }
        }

        void ANetStackMessageWorker::HandleMessage(RcvMessage *m) {
            // TODO(sunchao): 增加异步派发的逻辑？目前的话，按照设计思想，user应该自己在handler回调异步处理消息，不可以阻塞网络服务的IO线程。
            auto pCb = RcvMessage::LookupCallback(m->GetId());
            if (pCb) {
                pCb->first(m, pCb->second);
            } else {
                delete m;
            }
        }
    } // namespace net
} // namespace netty
