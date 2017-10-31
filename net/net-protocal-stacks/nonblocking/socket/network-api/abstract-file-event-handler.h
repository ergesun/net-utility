/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
#define NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H

#include "../../../../../common/common-def.h"
#include "../../../../../common/reference-counter.h"

#include "../../ievent-handler.h"
#include "file-descriptor.h"
#include "../event-drivers/event-worker.h"

namespace netty {
    namespace net {
        class ANetStackMessageWorker;
        class AFileEventHandler : public common::ReferenceCounter, public IEventHandler {
        public:
            AFileEventHandler() : common::ReferenceCounter(1) {}
            explicit AFileEventHandler(FileDescriptor *socketDesc) : common::ReferenceCounter(1), m_socketDesc(socketDesc) {}

            virtual ~AFileEventHandler() = default;
            /**
             * 必须先调用此函数进行初始化。
             * @return
             */
            virtual bool Initialize() = 0;

            inline FileDescriptor* GetSocketDescriptor() {
                return m_socketDesc;
            }

            inline void SetOwnWorker(EventWorker* ew) {
                m_pOwnEvWorker = ew;
            }

            inline EventWorker* GetOwnWorker() {
                return m_pOwnEvWorker;
            }

            void Release() final override {
                common::ReferenceCounter::Release();
                if (0 == common::ReferenceCounter::GetRef()) {
                    if (LIKELY(m_pOwnEvWorker)) {
                        m_pOwnEvWorker->AddExternalEpDelEvent(this);
                        m_pOwnEvWorker->Wakeup();
                    }
                }
            }

            virtual ANetStackMessageWorker* GetStackMsgWorker() = 0;

        protected:
            inline void SetSocketDescriptor(FileDescriptor *psd) {
                m_socketDesc = psd;
            }

        private:
            FileDescriptor *m_socketDesc = nullptr;
            EventWorker    *m_pOwnEvWorker = nullptr;
        };
    }
}

#endif //NET_CORE_SOCKETAPI_SOCKET_EVENT_HANDLER_H
