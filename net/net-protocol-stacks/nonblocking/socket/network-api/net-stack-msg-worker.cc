/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "../../../../../common/mem-pool.h"
#include "../../../../../common/buffer.h"
#include "../../../../../common/common-utils.h"
#include "../../../../rcv-message.h"

#include "abstract-file-event-handler.h"
#include "../../socket/event-drivers/event-worker.h"

#include "net-stack-msg-worker.h"

namespace netty {
    namespace net {
        std::function<void(RcvMessage*)> ANetStackMessageWorker::s_release_rm_handle = std::bind(&ANetStackMessageWorker::release_rcv_message,
                                                                                                 std::placeholders::_1);
        ANetStackMessageWorker::ANetStackMessageWorker(AFileEventHandler *eventHandler, common::MemPool *memPool,
                                                       NotifyMessageCallbackHandler msgCallbackHandler, uint32_t maxCacheMessageCnt) {
            m_pEventHandler = eventHandler;
            m_pMemPool = memPool;
            m_bqMessages = new common::BlockingQueue<SndMessage*>(maxCacheMessageCnt);
            auto size = RcvMessage::HeaderSize();
            auto headerMemObj = m_pMemPool->Get(size);
            m_pHeaderBuffer = common::CommonUtils::GetNewBuffer(headerMemObj, size);
            m_msgCallback = std::move(msgCallbackHandler);
        }

        ANetStackMessageWorker::~ANetStackMessageWorker() {
            SndMessage *sm = nullptr;
            while (m_bqMessages->TryPop(sm)) {
                DELETE_PTR(sm);
            }

            DELETE_PTR(m_bqMessages);
            DELETE_PTR(m_pHeaderBuffer);
        }

        bool ANetStackMessageWorker::SendMessage(SndMessage *m) {
            auto previousSize = m_bqMessages->Size();
            auto rc = m_bqMessages->TryPush(m);
            if (rc && 0 == previousSize) {
                // 我们无法保证事件管理器会绝对有写事件。比如事件管理器是epoll且为边缘触发，如果没有读事件，并且发送缓冲区未满，
                // 那么之后epoll是不会返回写事件的，那么这新加入的数据就无法发送出去了，所以此处需要唤醒epoll并添加外部事件。
                auto ew = m_pEventHandler->GetOwnWorker();
                NetEvent writeEvent = {
                    .eh = m_pEventHandler,
                    .mask = EVENT_WRITE
                };
                ew->AddExternalRWOpEvent(writeEvent);
                ew->Wakeup();
            }

            return rc;
        }

        void ANetStackMessageWorker::HandleMessage(NotifyMessage *m) {
            // TODO(sunchao): 增加异步派发的逻辑？目前的话，按照设计思想，user应该自己在handler回调异步处理消息，不可以阻塞网络服务的IO线程。
            auto ssp_rmr = std::shared_ptr<NotifyMessage>(m);
            if (m_msgCallback) {
                m_msgCallback(ssp_rmr);
            }
        }

        bool ANetStackMessageWorker::InsertMessage(SndMessage *m) {
            return m_bqMessages->TryPush(m);
        }

        void ANetStackMessageWorker::ClearMessage() {
            m_bqMessages->Clear();
        }

        RcvMessage* ANetStackMessageWorker::get_new_rcv_message(common::MemPool *mp, net_peer_info_t peerInfo,
                                                                 Message::Header h, common::Buffer *buffer) {
            auto rmMpo = mp->Get(sizeof(RcvMessage));
            auto rcvMessage = new(rmMpo->Pointer()) RcvMessage(rmMpo, mp, std::move(peerInfo), h, buffer);
            return rcvMessage;
        }

        void ANetStackMessageWorker::release_rcv_message(RcvMessage *rm) {
            rm->~RcvMessage();
        }
    } // namespace net
} // namespace netty
