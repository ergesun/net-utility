/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "event-worker.h"
#include "factory.h"
#include "../../../../../common/common-utils.h"
#include "../network-api/posix/local/simple-read-event-handler.h"

namespace netty {
    namespace net {
        EventWorker::EventWorker(uint32_t maxEvents, NonBlockingEventModel m) {
            m_vDriverInternalEvents.resize(maxEvents);
            m_pEventDriver = EventDriverFactory::GetNewDriver(m);
            m_pEventDriver->Init(maxEvents);
            int fds[2];
            if (pipe(fds) < 0) {
                auto err = errno;
                std::cerr << __func__ << " can't create notify pipe with errmsg = " << strerror(err) << std::endl;
                throw std::runtime_error("Create notify pipe failed!");
            }

            m_notifyRecvFd = fds[0];
            m_notifySendFd = fds[1];
            auto r = common::CommonUtils::SetNonBlocking(m_notifyRecvFd);
            if (r < 0) {
                auto err = errno;
                std::cerr << __func__ << " can't set notify pipe non-blocking with errmsg = " << strerror(err) << std::endl;
                throw std::runtime_error("Set notify pipe non-blocking failed!");
            }

            r = common::CommonUtils::SetNonBlocking(m_notifySendFd);
            if (r < 0) {
                auto err = errno;
                std::cerr << __func__ << " can't set notify pipe non-blocking with errmsg = " << strerror(err) << std::endl;
                throw std::runtime_error("Set notify pipe non-blocking failed!");
            }

            m_pLocalReadEventHandler = new PosixLocalReadEventHandler(m_notifyRecvFd);
            if (!m_pLocalReadEventHandler->Initialize()) {
                throw std::runtime_error("cannot initialize PosixLocalReadEventHandler.");
            }
            m_pEventDriver->AddEvent(m_pLocalReadEventHandler, EVENT_NONE, EVENT_READ);
        }

        EventWorker::~EventWorker() {
            DELETE_PTR(m_pEventDriver);
            DELETE_PTR(m_pLocalReadEventHandler);
            close(m_notifySendFd);
        }

        void EventWorker::Wakeup() {
            char buf = 'w';
            // wake up "event_wait"
            auto n = ::write(m_notifySendFd, &buf, sizeof(buf));
            if (n < 0) {
                std::cerr << __func__ << " write notify pipe failed: " << strerror(errno) << std::endl;
                throw std::runtime_error("Write notify pipe failed!");
            }

            fsync(m_notifySendFd);
        }
    }  // namespace net
}  // namespace netty
