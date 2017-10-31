/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include <stdexcept>

#include "simple-read-event-handler.h"

namespace netty {
    namespace net {
        PosixLocalReadEventHandler::PosixLocalReadEventHandler(int fd) : m_fd(fd) {
            net_peer_info_t peerInfo;
            auto pFd = new FileDescriptor(fd, peerInfo);
            SetSocketDescriptor(pFd);
        }

        PosixLocalReadEventHandler::~PosixLocalReadEventHandler() {
            auto fd = GetSocketDescriptor();
            close(m_fd);
            DELETE_PTR(fd);
        }

        bool PosixLocalReadEventHandler::Initialize() {
            return true;
        }

        bool PosixLocalReadEventHandler::HandleReadEvent() {
            char buf[1024];
            ssize_t r = 0;
            while (true) {
                r = read(m_fd, buf, sizeof(buf));
                if (r < 0) {
                    auto err = errno;
                    if (EAGAIN == err) {
                        break;
                    } else if (EINTR == err) {
                        continue;
                    } else {
                        std::cerr << __func__ << ": read err with errmsg = " << strerror(err) << std::endl;
                        throw std::runtime_error("Read err!");
                    }
                } else if (0 == r) {
                    auto err = errno;
                    std::cerr << __func__ << ": read err with errmsg = " << strerror(err) << std::endl;
                    throw std::runtime_error("Read err!");
                }
            }

            return true;
        }

        bool PosixLocalReadEventHandler::HandleWriteEvent() {
            throw std::runtime_error("Not support -- PosixLocalReadEventHandler::HandleWriteEvent!");
        }

        ANetStackMessageWorker *PosixLocalReadEventHandler::GetStackMsgWorker() {
            throw std::runtime_error("Not support -- PosixLocalReadEventHandler::GetStackMsgWorker!");
        }
    } // namespace net
} // namespace netty
