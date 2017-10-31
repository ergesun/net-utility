/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "common-def.h"

#include "reference-counter.h"

namespace netty {
    namespace common {
        void ReferenceCounter::AddRef() {
            atomic_addone_and_fetch(&m_iRef);
        }

        void ReferenceCounter::Release() {
            atomic_subone_and_fetch(&m_iRef);
        }

        void ReferenceCounter::SetRef(int32_t val) {
            atomic_fetch_and_set(&m_iRef, val);
        }

        int32_t ReferenceCounter::GetRef() {
            return atomic_fetch(&m_iRef);
        }
    }
}
