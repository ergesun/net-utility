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
            m_rcvState = NetWorkerState::StartToRcvHeader;                                                          \
            auto peer = m_pEventHandler->GetSocketDescriptor()->GetLogicPeerInfo();                                 \
            auto rcvMessage = get_new_rcv_message(m_pMemPool, peer, m_header, m_payloadBuffer);                     \
            if (LIKELY(ConnectionState::Connected == m_connState)) {                                                \
                auto mnm = new MessageNotifyMessage(rcvMessage, s_release_rm_handle);                               \
                HandleMessage(mnm);                                                                                 \
            } else {                                                                                                \
                handshake(rcvMessage);                                                                              \
            }                                                                                                       \
        } else {                                                                                                    \
            m_rcvState = NetWorkerState::RcvingPayload;                                                             \
            interrupt = true;                                                                                       \
        }

#define ProcessAfterRcvPayload()                                                                                    \
        if (0 == n) {                                                                                               \
            rc = false;                                                                                             \
            NotifyWorkerPeerClosedMessage();                                                                        \
        } else if (n > 0) {                                                                                         \
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
                net_local_info_t nlt = {
                    {
                        .addr = m_pEventHandler->GetSocketDescriptor()->GetRealPeerInfo().nat.addr.c_str(),
                        .port = m_iLogicPort
                    },
                    .sp = SocketProtocal::Tcp
                };
                auto crm = new ConnectRequestMessage(m_pMemPool, std::move(nlt));
                //fprintf(stderr, "connect msg len = %d\n", crm->GetDerivePayloadLength());
                this->SendMessage(crm);
                m_connState = ConnectionState::ConnectSe;
            }

            std::unique_lock<std::mutex> l(m_initWaitMtx);
            while (!m_bConnHandShakeCompleted) {
                m_initWaitCv.wait(l);
            }

            return ConnectionState::Connected == m_connState;
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

        void PosixTcpNetStackWorker::handshake(RcvMessage *rm) {
            switch (m_connState) {
                case ConnectionState::ConnectSe: {
                    // just client
                    auto buffer = rm->GetDataBuffer();
                    auto res = ByteOrderUtils::ReadUInt16(buffer->Pos);
                    if (ConnectResponseMessage::Status::OK != (ConnectResponseMessage::Status)res) {
                        buffer->Pos += sizeof(uint16_t);
                        auto whatLen = buffer->AvailableLength();
                        auto whatMpo = m_pMemPool->Get((uint32_t)(whatLen + 1));
                        auto whatPtr = whatMpo->Pointer();
                        memcpy(whatPtr, buffer->Pos, (size_t)whatLen);
                        *(whatPtr + whatLen) = 0;
                        whatMpo->Put();
                        fprintf(stderr, "%s", whatPtr);
                        m_bConnHandShakeCompleted = true;
                        m_initWaitCv.notify_one();

                    } else {
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                        this->SendMessage(crm);
                        m_connState = ConnectionState::WaitLastACK;
                    }

                    s_release_rm_handle(rm);
                    break;
                }
                case ConnectionState::WaitLastACK: {
                    // just client
                    auto buffer = rm->GetDataBuffer();
                    auto res = ByteOrderUtils::ReadUInt16(buffer->Pos);
                    s_release_rm_handle(rm);
                    if (ConnectResponseMessage::Status::OK == (ConnectResponseMessage::Status)res) {
                        if (m_onLogicConnect(m_pEventHandler)) {
                            // 如果服务端返回的不是成功，则这里不会赋值，那么client的initialize就会失败，进而收回handler及fd。
                            m_connState = ConnectionState::Connected;
                        }
                    }

                    m_bConnHandShakeCompleted = true;
                    m_initWaitCv.notify_one();
                    break;
                }
                case ConnectionState::Connecting: {
                    // just server
                    auto buffer = rm->GetDataBuffer();
                    if (UNLIKELY(11 >= buffer->AvailableLength())) { // 长度有问题至少也的2 + 2 + 7(0:0:0:0);
                        fprintf(stderr, "client connecting buffer is corrupt.\n");
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR, "");
                        this->SendMessage(crm);
                        m_bConnHandShakeCompleted = true;
                        s_release_rm_handle(rm);
                        m_initWaitCv.notify_one();
                    } else {
                        auto sp = ByteOrderUtils::ReadUInt16(buffer->Pos);
                        buffer->Pos += sizeof(uint16_t);
                        auto port = ByteOrderUtils::ReadUInt16(buffer->Pos);
                        buffer->Pos += sizeof(uint16_t);
                        auto addrLen = buffer->AvailableLength();
                        auto addrMpo = m_pMemPool->Get((uint32_t)(addrLen + 1));
                        auto addrPtr = addrMpo->Pointer();
                        memcpy(addrPtr, buffer->Pos, (size_t)addrLen);
                        *(addrPtr + addrLen) = 0;
                        std::string addrStr = addrPtr;
                        net_peer_info_t npt = {
                            {
                                .addr = std::move(addrStr),
                                .port = port
                            },
                            .sp = (SocketProtocal)sp
                        };

                        addrMpo->Put();
                        // set peer info to logic info.
                        this->GetEventHandler()->GetSocketDescriptor()->SetLogicPeerInfo(std::move(npt));
                        auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                        this->SendMessage(crm);
                        m_connState = ConnectionState::ConnectingRe;
                        s_release_rm_handle(rm);
                    }

                    break;
                }
                case ConnectionState::ConnectingRe: {
                    // just server
                    ConnectResponseMessage *crm = nullptr;
                    s_release_rm_handle(rm);
                    if (m_onLogicConnect(m_pEventHandler)) {
                        m_connState = ConnectionState::Connected;
                        crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::OK, "");
                    } else {
                        crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR, "");
                    }

                    this->SendMessage(crm);
                    m_bConnHandShakeCompleted = true;
                    m_initWaitCv.notify_one();
                    break;
                }
                default: {
                    fprintf(stderr, "handshake recv unexpected state %d\n", (int)m_connState);
                    auto crm = new ConnectResponseMessage(m_pMemPool, ConnectResponseMessage::Status::ERROR,
                                                          "handshake recv unexpected state.");
                    this->SendMessage(crm);
                    s_release_rm_handle(rm);
                }
            }
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_closed_by_peer_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::ClosedByPeer, std::move(msg));
        }

        WorkerNotifyMessage* PosixTcpNetStackWorker::get_broken_worker_message(std::string &&msg) {
            return new WorkerNotifyMessage(WorkerNotifyMessageCode::Error, std::move(msg));
        }
    } // namespace net
} // namespace netty
