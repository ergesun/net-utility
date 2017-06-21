/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../../../../../../../common/buffer.h"
#include "../../../../../../rcv-message.h"

#include "net-stack-worker.h"

#define NotifyWorkerBrokenMessage()                                                                                 \
        auto wnm = get_broken_worker_message(strerror(err));                                                        \
        HandleMessage(wnm);

#define NotifyWorkerPeerClosedMessage()                                                                             \
        auto wnm = get_closed_by_peer_worker_message("closed by peer.");                                            \
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
            m_pHeaderBuffer->RecvN((uint32_t)n);                                                                    \
            ParseHeader();                                                                                          \
        } else {                                                                                                    \
            interrupt = true;                                                                                       \
            if (EAGAIN != err) {                                                                                    \
                rc = false;                                                                                         \
                NotifyWorkerBrokenMessage();                                                                        \
            }                                                                                                       \
        }

#define CheckPayload()                                                                                              \
        if (m_payloadBuffer->AvailableLength() == m_header.len) {                                                   \
            m_rcvState = NetWorkerState::StartToRcvHeader;                                                          \
            auto rcvMessage = get_new_rcv_message(m_pMemPool, m_header, m_payloadBuffer);                           \
            auto mnm = new MessageNotifyMessage(rcvMessage, s_release_rm_handle);                                   \
            HandleMessage(mnm);                                                                                     \
        } else {                                                                                                    \
            m_rcvState = NetWorkerState::RcvingPayload;                                                             \
            interrupt = true;                                                                                       \
        }

#define ProcessAfterRcvPayload()                                                                                    \
        if (0 == n) {                                                                                               \
            rc = false;                                                                                             \
            NotifyWorkerPeerClosedMessage();                                                                        \
        } else if (n > 0) {                                                                                         \
            m_payloadBuffer->RecvN((uint32_t)n);                                                                    \
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
        PosixTcpNetStackWorker::PosixTcpNetStackWorker(AFileEventHandler *eventHandler, common::MemPool *memPool,
                                                       PosixTcpClientSocket *socket,
                                                       NotifyMessageCallbackHandler msgCallbackHandler)
            : ANetStackMessageWorker(eventHandler, memPool, msgCallbackHandler), m_pSocket(socket) {}

        PosixTcpNetStackWorker::~PosixTcpNetStackWorker() {
            if (m_payloadBuffer) {
                Message::PutBuffer(m_payloadBuffer);
            }

            if (m_pSendingBuffer) {
                Message::PutBuffer(m_pSendingBuffer);
            }
        }

        bool PosixTcpNetStackWorker::Recv() {
            int err;
            bool rc = true;
            bool interrupt = false;
            while (rc && !interrupt) {
                switch (m_rcvState) {
                    case NetWorkerState::StartToRcvHeader:{
                        m_pHeaderBuffer->BZero();
                        auto n = m_pSocket->Read(m_pHeaderBuffer->Start, (size_t)m_pHeaderBuffer->TotalLength(), err);
                        ProcessAfterRcvHeader();

                        break;
                    }
                    case NetWorkerState::RcvingHeader:{
                        void *pos;
                        if (LIKELY(m_pHeaderBuffer->Last)) {
                            pos = m_pHeaderBuffer->Last + 1;
                        } else {
                            pos = m_pHeaderBuffer->Start;
                        }
                        auto n = m_pSocket->Read(pos, m_pHeaderBuffer->UnusedSize(), err);
                        ProcessAfterRcvHeader();

                        break;
                    }
                    case NetWorkerState::StartToRcvPayload:{
                        auto mpo = m_pMemPool->Get(m_header.len);
                        m_payloadBuffer = Message::GetNewBuffer(mpo, m_header.len);
                        auto n = m_pSocket->Read(m_payloadBuffer->Start, (size_t)m_payloadBuffer->TotalLength(), err);
                        ProcessAfterRcvPayload();

                        break;
                    }
                    case NetWorkerState::RcvingPayload:{
                        void *pos;
                        if (m_payloadBuffer->Last) {
                            pos = m_payloadBuffer->Last + 1;
                        } else {
                            pos = m_payloadBuffer->Start;
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
                    auto n = m_pSocket->Write(m_pSendingBuffer->Pos, size, err);
                    if (0 == n) {
                        rc = false;
                        Put_Send_Buffer();
                        NotifyWorkerPeerClosedMessage();
                    } else if (0 < n) {
                        m_pSendingBuffer->SendN(uint32_t(n));
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

#undef Put_Send_Buffer
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_closed_by_peer_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::ClosedByPeer, std::move(msg));
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_broken_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::Error, std::move(msg));
        }
    } // namespace net
} // namespace netty
