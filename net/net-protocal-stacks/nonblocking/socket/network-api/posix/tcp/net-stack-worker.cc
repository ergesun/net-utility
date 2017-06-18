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
                            m_pHeaderBuffer->Pos = m_pHeaderBuffer->Start;
                            m_pHeaderBuffer->Last = m_pHeaderBuffer->Start + n - 1;
                        }

                        ParseHeader();
                        break;
                    }
                    case NetWorkerState::RcvingHeader:{
                        auto n = m_pSocket->Read(m_pHeaderBuffer->Last + 1, m_pHeaderBuffer->UnusedSize());
                        if (n > 0) {
                            m_pHeaderBuffer->Last = m_pHeaderBuffer->Last + n;
                        }

                        ParseHeader();
                        break;
                    }
                    case NetWorkerState::StartToRcvPayload:{
                        auto mpo = m_pMemPool->Get(m_header.len);
                        m_payloadBuffer = Message::GetNewBuffer(mpo, m_header.len);
                        auto n = m_pSocket->Read(m_payloadBuffer->Start, (size_t)m_payloadBuffer->TotalLength());
                        if (n > 0) {
                            m_payloadBuffer->Pos = m_payloadBuffer->Start;
                            m_payloadBuffer->Last = m_payloadBuffer->Start + n - 1;
                        }

                        if (m_payloadBuffer->AvailableLength() == m_header.len) {
                            m_state = NetWorkerState::StartToRcvHeader;
                            interrupt = false;
                            RcvMessage *rcvMessage = new RcvMessage(m_pMemPool, m_header, m_payloadBuffer, NettyMsgCode::OK);
                            HandleMessage(rcvMessage);
                        } else {

                        }
                        break;
                    }
                    case NetWorkerState::RcvingPayload:{
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
