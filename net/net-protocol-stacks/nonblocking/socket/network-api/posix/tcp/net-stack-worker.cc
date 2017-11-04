/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../../../../../../../common/buffer.h"
#include "../../../../../../rcv-message.h"
#include "../../abstract-file-event-handler.h"

#include "net-stack-worker.h"
#include "connect-messages/connect-response-message.h"
#include "connect-messages/connect-request-message.h"

#define CheckAndInitBufferPos(b)                                                                                    \
        if (!b->GetPos()) {                                                                                         \
            b->SetPos(b->GetStart());                                                                               \
        }

#define NotifyWorkerBrokenMessage()                                                                                 \
        auto wnm = get_broken_worker_message(strerror(err));                                                        \
        HandleMessage(wnm);

#define NotifyWorkerPeerClosedMessage()                                                                             \
        std::stringstream ss;                                                                                       \
        ss << "Closed by peer = " << m_pEventHandler->GetSocketDescriptor()->GetRealPeerInfo().nat.addr << ":"      \
           << m_pEventHandler->GetSocketDescriptor()->GetRealPeerInfo().nat.port;                                   \
        auto wnm = get_closed_by_peer_worker_message(ss.str());                                                     \
        HandleMessage(wnm);

#define ParseHeader()                                                                                               \
        if (m_pHeaderBuffer->AvailableLength() == m_pHeaderBuffer->TotalLength()) {                                 \
            auto headerRc = RcvMessage::DecodeMsgHeader(m_pHeaderBuffer, &m_header);                                \
            if (!headerRc) {                                                                                        \
                std::cerr << "Decode message header failed!" << std::endl;                                          \
                rc = false;                                                                                         \
                NotifyWorkerBrokenMessage();                                                                        \
            } else {                                                                                                \
                m_rcvState = NetWorkerState::StartToRcvPayload;                                                     \
            }                                                                                                       \
        } else {                                                                                                    \
            interrupt = true;                                                                                       \
            m_rcvState = NetWorkerState::RcvingHeader;                                                              \
        }

#define ProcessAfterRcvHeader()                                                                                     \
        if (0 == n) {                                                                                               \
            rc = false;                                                                                             \
            NotifyWorkerPeerClosedMessage();                                                                        \
        } else if (n > 0) {                                                                                         \
            CheckAndInitBufferPos(m_pHeaderBuffer);                                                                 \
            m_pHeaderBuffer->MoveTailBack((uint32_t)n);                                                             \
            ParseHeader();                                                                                          \
        } else {                                                                                                    \
            interrupt = true;                                                                                       \
            if (EAGAIN != err) {                                                                                    \
                rc = false;                                                                                         \
                NotifyWorkerBrokenMessage();                                                                        \
            }                                                                                                       \
        }

#define CheckPayload()                                                                                              \
        if ((uint32_t)m_payloadBuffer->AvailableLength() == m_header.len) {                                         \
            if (breakWhenRecvOne) {                                                                                 \
                interrupt = true;                                                                                   \
            }                                                                                                       \
            m_rcvState = NetWorkerState::StartToRcvHeader;                                                          \
            auto peer = m_pEventHandler->GetSocketDescriptor()->GetLogicPeerInfo();                                 \
            auto rcvMessage = get_new_rcv_message(m_pMemPool, peer, m_header, m_payloadBuffer);                     \
            /* TODO(sunchao): 此指针需要加锁以防止析够后还会被handle message使用而导致崩溃。*/                            \
            if (LIKELY(ConnectionState::Connected == m_connState)) {                                                \
                auto mnm = new MessageNotifyMessage(rcvMessage, s_release_rm_handle);                               \
                HandleMessage(mnm);                                                                                 \
            } else {                                                                                                \
                handshake(rcvMessage);                                                                              \
            }                                                                                                       \
            m_payloadBuffer = nullptr;                                                                              \
        } else {                                                                                                    \
            m_rcvState = NetWorkerState::RcvingPayload;                                                             \
            interrupt = true;                                                                                       \
        }

#define ProcessAfterRcvPayload()                                                                                    \
        if (0 == n) {                                                                                               \
            rc = false;                                                                                             \
            NotifyWorkerPeerClosedMessage();                                                                        \
        } else if (n > 0) {                                                                                         \
            CheckAndInitBufferPos(m_payloadBuffer);                                                                 \
            m_payloadBuffer->MoveTailBack((uint32_t)n);                                                             \
            CheckPayload();                                                                                         \
        } else {                                                                                                    \
            interrupt = true;                                                                                       \
            if (EAGAIN != err) {                                                                                    \
                rc = false;                                                                                         \
                NotifyWorkerBrokenMessage();                                                                        \
            }                                                                                                       \
        }

namespace netty {
    namespace net {
        PosixTcpNetStackWorker::PosixTcpNetStackWorker(CreatorType ct, AFileEventHandler *eventHandler,
                                                       common::MemPool *memPool, PosixTcpClientSocket *socket,
                                                       NotifyMessageCallbackHandler msgCallbackHandler, uint16_t logicPort,
                                                       ConnectFunc logicConnect)
            : ANetStackMessageWorker(eventHandler, memPool, std::move(msgCallbackHandler)), m_ct(ct),
              m_pSocket(socket), m_iLogicPort(logicPort), m_onLogicConnect(std::move(logicConnect)) {}

        PosixTcpNetStackWorker::PosixTcpNetStackWorker(CreatorType ct, AFileEventHandler *eventHandler,
                                                       common::MemPool *memPool, PosixTcpClientSocket *socket,
                                                       NotifyMessageCallbackHandler msgCallbackHandler,
                                                       ConnectFunc logicConnect)
            : ANetStackMessageWorker(eventHandler, memPool, std::move(msgCallbackHandler)),
              m_ct(ct), m_pSocket(socket), m_iLogicPort(0), m_onLogicConnect(std::move(logicConnect)) {}

        PosixTcpNetStackWorker::~PosixTcpNetStackWorker() {
            m_bConnHandShakeCompleted = true;
            m_initWaitCv.notify_one();
            if (m_payloadBuffer) {
                Message::PutBuffer(m_payloadBuffer);
            }

            if (m_pSendingBuffer) {
                Message::PutBuffer(m_pSendingBuffer);
            }
        }

        // TODO(sunchao): 加个超时？
        bool PosixTcpNetStackWorker::Initialize() {
            m_connState = ConnectionState::Connecting;
            if (CreatorType::Client == m_ct) {
                auto crm = new ConnectRequestMessage(m_pMemPool, m_iLogicPort);
                if (UNLIKELY(!InsertMessage(crm))) {
                    return false;
                }

                if (!Send()) {
                    return false;
                }

                m_connState = ConnectionState::ConnectSe;
            }

            if (!Recv(true)) {
                return false;
            }

            std::unique_lock<std::mutex> l(m_initWaitMtx);
            while (!m_bConnHandShakeCompleted) {
                m_initWaitCv.wait(l);
            }

            return ConnectionState::Connected == m_connState;
        }

        bool PosixTcpNetStackWorker::Recv(bool breakWhenRecvOne) {
            int err;
            bool rc = true;
            bool interrupt = false;
            while (rc && !interrupt) {
                switch (m_rcvState) {
                    case NetWorkerState::StartToRcvHeader:{
                        m_pHeaderBuffer->BZero();
                        auto n = m_pSocket->Read(m_pHeaderBuffer->GetStart(), (size_t)m_pHeaderBuffer->TotalLength(), err);
                        ProcessAfterRcvHeader();
                        break;
                    }
                    case NetWorkerState::RcvingHeader:{
                        void *pos;
                        if (LIKELY(m_pHeaderBuffer->GetLast())) {
                            pos = m_pHeaderBuffer->GetLast() + 1;
                        } else {
                            pos = m_pHeaderBuffer->GetStart();
                        }

                        auto n = m_pSocket->Read(pos, m_pHeaderBuffer->UnusedSize(), err);
                        ProcessAfterRcvHeader();
                        break;
                    }
                    case NetWorkerState::StartToRcvPayload:{
                        auto mpo = m_pMemPool->Get(m_header.len);
                        m_payloadBuffer = Message::GetNewBuffer(mpo, m_header.len);
                        auto n = m_pSocket->Read(m_payloadBuffer->GetStart(), (size_t)m_payloadBuffer->TotalLength(), err);
                        ProcessAfterRcvPayload();
                        break;
                    }
                    case NetWorkerState::RcvingPayload:{
                        void *pos;
                        if (m_payloadBuffer->GetLast()) {
                            pos = m_payloadBuffer->GetLast() + 1;
                        } else {
                            pos = m_payloadBuffer->GetStart();
                        }
                        auto n = m_pSocket->Read(pos, m_payloadBuffer->UnusedSize(), err);
                        ProcessAfterRcvPayload();

                        break;
                    }
                }
            }

            return rc;
        }

        bool PosixTcpNetStackWorker::Send() {
#define Put_Send_Buffer()                          \
        Message::PutBuffer(m_pSendingBuffer);      \
        m_pSendingBuffer = nullptr;

            int err;
            size_t size;
            bool rc = true;
            bool interrupt = false;
            while (rc && !interrupt) {
                size = m_pSendingBuffer ? (size_t)m_pSendingBuffer->AvailableLength() : 0;
                if (0 < size) {
                    auto n = m_pSocket->Write(m_pSendingBuffer->GetPos(), size, err);
                    if (0 == n) {
                        rc = false;
                        Put_Send_Buffer();
                        NotifyWorkerPeerClosedMessage();
                    } else if (0 < n) {
                        m_pSendingBuffer->MoveHeadBack(uint32_t(n));
                        if (m_pSendingBuffer->AvailableLength() <= 0) {
                            Put_Send_Buffer();
                        }
                    } else {
                        interrupt = true;
                        if (EAGAIN != err) {
                            rc = false;
                            Put_Send_Buffer();
                            NotifyWorkerBrokenMessage();
                        }
                    }
                } else {
                    SndMessage *sm;
                    if (m_bqMessages->TryPop(sm)) {
                        m_pSendingBuffer = sm->Encode();
                        DELETE_PTR(sm);
                    } else {
                        interrupt = true;
                    }
                }
            }

            return rc;
#undef Put_Send_Buffer
        }

        /**
         * 握手期间需要同步的系统调用send/recv以便控制握手，所以这之前不会把fd交给事件管理器。
         * @param rm
         */
        void PosixTcpNetStackWorker::handshake(RcvMessage *rm) {
#define COMPLETE_AND_FIRE()                   \
        m_bConnHandShakeCompleted = true;     \
        m_initWaitCv.notify_one();

            switch (m_connState) {
                case ConnectionState::ConnectSe: {
                    // just client
                    auto buffer = rm->GetDataBuffer();
                    auto res = ByteOrderUtils::ReadUInt16(buffer->GetPos());
                    if (ConnectResponseMessage::Status::OK != (ConnectResponseMessage::Status)res) {
                        buffer->MoveHeadBack(sizeof(uint16_t));
                        auto whatLen = buffer->AvailableLength();
                        auto whatMpo = m_pMemPool->Get((uint32_t)(whatLen + 1));
                        auto whatPtr = whatMpo->Pointer();
                        memcpy(whatPtr, buffer->GetPos(), (size_t)whatLen);
                        *(whatPtr + whatLen) = 0;
                        whatMpo->Put();
                        fprintf(stderr, "%s", whatPtr);
                        COMPLETE_AND_FIRE();
                    } else {
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                        bool ok;
                        if (UNLIKELY(!(ok = InsertMessage(crm)))) {
                            COMPLETE_AND_FIRE();
                        } else if (!(ok = Send())) {
                            COMPLETE_AND_FIRE();
                        }

                        if (ok) {
                            m_connState = ConnectionState::WaitLastACK;
                            if (!Recv(true)) {
                                COMPLETE_AND_FIRE();
                            }
                        }
                    }

                    s_release_rm_handle(rm);
                    break;
                }
                case ConnectionState::WaitLastACK: {
                    // just client
                    auto buffer = rm->GetDataBuffer();
                    auto res = ByteOrderUtils::ReadUInt16(buffer->GetPos());
                    s_release_rm_handle(rm);
                    if (ConnectResponseMessage::Status::OK == (ConnectResponseMessage::Status)res) {
                        if (m_onLogicConnect(m_pEventHandler)) {
                            // 如果服务端返回的不是成功，则这里不会赋值，那么client的initialize就会失败，进而收回handler及fd。
                            m_connState = ConnectionState::Connected;
                        }
                    }

                    COMPLETE_AND_FIRE();
                    break;
                }
                case ConnectionState::Connecting: {
                    // just server
                    auto buffer = rm->GetDataBuffer();
                    if (UNLIKELY(2 != buffer->AvailableLength())) { // 长度有问题至少也的2;
                        fprintf(stderr, "client connecting buffer is corrupt.\n");
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR, "");
                        InsertMessage(crm);
                        Send();
                        s_release_rm_handle(rm);
                        COMPLETE_AND_FIRE();
                    } else {
                        auto port = ByteOrderUtils::ReadUInt16(buffer->GetPos());
                        std::string addrStr = GetEventHandler()->GetSocketDescriptor()->GetRealPeerInfo().nat.addr;
                        net_peer_info_t npt = {
                            {
                                .addr = std::move(addrStr),
                                .port = port
                            },
                            .sp = SocketProtocol::Tcp
                        };

                        // set peer info to logic info.
                        this->GetEventHandler()->GetSocketDescriptor()->SetLogicPeerInfo(std::move(npt));
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                        bool ok;
                        if (UNLIKELY(!(ok = InsertMessage(crm)))) {
                            COMPLETE_AND_FIRE();
                        } else if (!(ok = Send())) {
                            COMPLETE_AND_FIRE();
                        }
                        if (ok) {
                            m_connState = ConnectionState::ConnectingRe;
                            if (!Recv(true)) {
                                COMPLETE_AND_FIRE();
                            }
                        }

                        s_release_rm_handle(rm);
                    }

                    break;
                }
                case ConnectionState::ConnectingRe: {
                    // just server
                    ConnectResponseMessage *crm = nullptr;
                    s_release_rm_handle(rm);
                    bool ok = false;
                    if (m_onLogicConnect(m_pEventHandler)) {
                        ok = true;
                        crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                    } else {
                        crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR, "");
                    }

                    if (!InsertMessage(crm)) {
                        ok = false;
                    } else if (!Send()) {
                        ok = false;
                    }

                    if (ok) {
                        m_connState = ConnectionState::Connected;
                    }

                    COMPLETE_AND_FIRE();
                    break;
                }
                default: {
                    fprintf(stderr, "handshake recv unexpected state %d\n", (int)m_connState);
                    auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR,
                                                          "handshake recv unexpected state.");
                    if (InsertMessage(crm)) {
                        Send();
                    }

                    COMPLETE_AND_FIRE();
                    s_release_rm_handle(rm);
                }
            }
#undef COMPLETE_AND_FIRE
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_closed_by_peer_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::ClosedByPeer, std::move(msg));
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_broken_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::Error, std::move(msg));
        }
    } // namespace net
} // namespace netty
