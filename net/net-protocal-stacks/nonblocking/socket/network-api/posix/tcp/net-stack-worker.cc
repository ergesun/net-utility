/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "../../../../../../../common/buffer.h"

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
                        auto n = m_pSocket->Read(m_pHeaderBuffer->Last + 1, (size_t)(m_pHeaderBuffer->End - m_pHeaderBuffer->Last));
                        if (n > 0) {
                            m_pHeaderBuffer->Last = m_pHeaderBuffer->Last + n;
                        }

                        ParseHeader();
                        break;
                    }
                    case NetWorkerState::StartToRcvPayload:{

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
