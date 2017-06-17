/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "event-worker.h"
#include "factory.h"

namespace netty {
    namespace net {
        EventWorker::EventWorker(uint32_t maxEvents, NonBlockingEventModel m) {
            m_vEvents.resize(maxEvents);
            m_pEventDriver = EventDriverFactory::GetNewDriver(m);
            m_pEventDriver->Init(maxEvents);
        }

        EventWorker::~EventWorker() {
            DELETE_PTR(m_pEventDriver);
        }
    }  // namespace net
}  // namespace netty
