/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "simple-read-event-handler.h"

namespace netty {
    namespace net {
        PosixLocalReadEventHandler::~PosixLocalReadEventHandler() {
            close(m_fd);
        }

        bool PosixLocalReadEventHandler::HandleReadEvent() {
            char buf[1024];
            int r = 0;
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
            throw std::runtime_error("Not support!");
        }

        ANetStackMessageWorker *PosixLocalReadEventHandler::GetStackMsgWorker() {
            throw std::runtime_error("Not support!");
        }
    } // namespace net
} // namespace netty
