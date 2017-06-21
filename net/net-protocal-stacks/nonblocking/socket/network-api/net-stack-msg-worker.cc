/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../../../../../common/mem-pool.h"
#include "../../../../../common/buffer.h"
#include "../../../../../common/common-utils.h"
#include "../../../../rcv-message.h"

#include "abstract-file-event-handler.h"

#include "net-stack-msg-worker.h"


namespace netty {
    namespace net {
        ANetStackMessageWorker::ANetStackMessageWorker(AFileEventHandler *eventHandler, common::MemPool *memPool,
                                                       MsgCallbackHandler msgCallbackHandler, uint32_t maxCacheMessageCnt) {
            m_pEventHandler = eventHandler;
            m_pMemPool = memPool;
            m_bqMessages = new common::BlockingQueue<SndMessage*>(maxCacheMessageCnt);
            auto size = RcvMessage::HeaderSize();
            auto headerMemObj = m_pMemPool->Get(size);
            m_pHeaderBuffer = common::CommonUtils::GetNewBuffer(headerMemObj, size);
            m_msgCallback = msgCallbackHandler;
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
                release_rcv_message(rm);
            };
            auto rmRef = new RcvMessageRef(m, release_rm_handle);
            auto ssp_rmr = std::shared_ptr<RcvMessageRef>(rmRef);
            if (m_msgCallback) {
                m_msgCallback(ssp_rmr);
            }
        }

        RcvMessage * ANetStackMessageWorker::get_new_rcv_message(common::MemPool *mp, Message::Header h,
                                                                 common::Buffer *buffer, NettyMsgCode rc) {
            auto rmMpo = mp->Get(sizeof(RcvMessage));
            auto rcvMessage = new(rmMpo->Pointer()) RcvMessage(rmMpo, mp, h, buffer, rc);
            return rcvMessage;
        }

        void ANetStackMessageWorker::release_rcv_message(RcvMessage *rm) {
            rm->~RcvMessage();
        }
    } // namespace net
} // namespace netty
