/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../../../../../../../common/buffer.h"
#include "../../../../../../rcv-message.h"

#include "net-stack-worker.h"


#define ParseHeader()                                                                                       \
        if (m_pHeaderBuffer->AvailableLength() == m_pHeaderBuffer->TotalLength()) {                         \
            auto headerRc = RcvMessage::DecodeMsgHeader(m_pHeaderBuffer, &m_header);                        \
            if (!headerRc) {                                                                                \
                std::cerr << "Decode message header failed!" << std::endl;                                  \
                rc = false;                                                                                 \
            } else {                                                                                        \
                interrupt = false;                                                                          \
                m_state = NetWorkerState::StartToRcvPayload;                                                \
            }                                                                                               \
        } else {                                                                                            \
            interrupt = true;                                                                               \
            m_state = NetWorkerState::RcvingHeader;                                                         \
        }

#define CheckPayload()                                                                                      \
        if (m_payloadBuffer->AvailableLength() == m_header.len) {                                           \
            m_state = NetWorkerState::StartToRcvHeader;                                                     \
            interrupt = false;                                                                              \
            auto rcvMessage = get_new_rcv_message(m_pMemPool, m_header, m_payloadBuffer, NettyMsgCode::OK); \
            HandleMessage(rcvMessage);                                                                      \
        } else {                                                                                            \
            m_state = NetWorkerState::RcvingPayload;                                                        \
            interrupt = true;                                                                               \
        }

namespace netty {
    namespace net {
        PosixTcpNetStackWorker::PosixTcpNetStackWorker(common::MemPool *memPool, PosixTcpClientSocket *socket)
            : ANetStackMessageWorker(memPool), m_pSocket(socket) {}

        PosixTcpNetStackWorker::~PosixTcpNetStackWorker() {
            if (m_payloadBuffer) {
                Message::PutBuffer(m_payloadBuffer);
            }
        }

        bool PosixTcpNetStackWorker::Recv() {
            bool rc = true;
            bool interrupt = false;
            while (rc && !interrupt) {
                switch (m_state) {
                    case NetWorkerState::StartToRcvHeader:{
                        m_pHeaderBuffer->BZero();
                        auto n = m_pSocket->Read(m_pHeaderBuffer->Start, (size_t)m_pHeaderBuffer->TotalLength());
                        if (n > 0) {
                            m_pHeaderBuffer->RecvN((uint32_t)n);
                        }

                        ParseHeader();
                        break;
                    }
                    case NetWorkerState::RcvingHeader:{
                        void *pos;
                        if (LIKELY(m_pHeaderBuffer->Last)) {
                            pos = m_pHeaderBuffer->Last + 1;
                        } else {
                            pos = m_pHeaderBuffer->Start;
                        }
                        auto n = m_pSocket->Read(pos, m_pHeaderBuffer->UnusedSize());
                        if (n > 0) {
                            m_pHeaderBuffer->RecvN((uint32_t)n);
                        }

                        ParseHeader();
                        break;
                    }
                    case NetWorkerState::StartToRcvPayload:{
                        auto mpo = m_pMemPool->Get(m_header.len);
                        m_payloadBuffer = Message::GetNewBuffer(mpo, m_header.len);
                        auto n = m_pSocket->Read(m_payloadBuffer->Start, (size_t)m_payloadBuffer->TotalLength());
                        if (n > 0) {
                            m_payloadBuffer->RecvN((uint32_t)n);
                        }

                        CheckPayload();
                        break;
                    }
                    case NetWorkerState::RcvingPayload:{
                        void *pos;
                        if (m_payloadBuffer->Last) {
                            pos = m_payloadBuffer->Last + 1;
                        } else {
                            pos = m_payloadBuffer->Start;
                        }
                        auto n = m_pSocket->Read(pos, m_payloadBuffer->UnusedSize());
                        if (n > 0) {
                            m_payloadBuffer->RecvN((uint32_t)n);
                        }

                        CheckPayload();
                        break;
                    }
                }
            }

            return rc;
        }

        bool PosixTcpNetStackWorker::Send() {

        }
    } // namespace net
} // namespace netty
