/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../../../../../common/mem-pool.h"
#include "../../../../../common/buffer.h"
#include "../../../../../common/common-utils.h"
#include "../../../../rcv-message.h"

#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {

        common::spin_lock_t ANetStackMessageWorker::s_cbLock = UNLOCKED;
        std::unordered_map<Message::Id, MsgCallback> ANetStackMessageWorker::s_callbacks = std::unordered_map<Message::Id, MsgCallback>();
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
            static auto release_rm_handle = [](RcvMessage *rm) -> void {
                DELETE_PTR(rm);
            };
            auto rmRef = new RcvMessageRef(m, release_rm_handle);
            auto ssp_rmr = std::shared_ptr<RcvMessageRef>(rmRef);
            auto pCb = ANetStackMessageWorker::lookup_callback(m->GetId());
            if (pCb) {
                pCb->first(ssp_rmr, pCb->second);
            } else {
                delete m;
            }
        }

        MsgCallback* ANetStackMessageWorker::lookup_callback(Message::Id id) {
            common::SpinLock l(&s_cbLock);
            auto cbIter = s_callbacks.find(id);
            if (LIKELY(cbIter != s_callbacks.end())) {
                return &cbIter->second;
            }

            return nullptr;
        }

        void ANetStackMessageWorker::add_callback(Message::Id id, MsgCallback cb) {
            common::SpinLock l(&s_cbLock);
            s_callbacks[id] = cb;
        }

        void ANetStackMessageWorker::remove_callback(Message::Id id) {
            common::SpinLock l(&s_cbLock);
            s_callbacks.erase(id);
        }
    } // namespace net
} // namespace netty
