/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "common-def.h"
#include "spin-lock.h"

namespace common {
SpinLock::SpinLock(spin_lock_t* const sl) : m_psl(sl) {
    assert(sl);
    *sl = UNLOCKED;
    Lock();
}

SpinLock::SpinLock(spin_lock_t *const sl, bool) : m_psl(sl) {
    assert(sl);
    *sl = UNLOCKED;
}

SpinLock::SpinLock(spin_lock_t *const sl, uint8_t spin) : m_psl(sl), m_iSpin(1 << spin) {
    assert(sl);
    *sl = UNLOCKED;
    Lock();
}

SpinLock::SpinLock(spin_lock_t *const sl, uint8_t spin, bool) : m_psl(sl), m_iSpin(1 << spin) {
    assert(sl);
    *sl = UNLOCKED;
}

SpinLock::~SpinLock() {
    if (m_bOwnLock) {
        Unlock();
    }
}

void SpinLock::Lock() {
    if (m_bOwnLock) {
        throw new std::runtime_error("spin_lock_t is locked, cannot lock it again.");
    }

    int i, n;
    for (;;) {
        if (atomic_cas(m_psl, UNLOCKED, LOCKED)) {
            m_bOwnLock = true;
            return;
        }

        for (n = 1; n < m_iSpin; n <<= 1) {
            for (i = 0; i < n; ++i) {
                soft_yield_cpu();
            }

            if (atomic_cas(m_psl, UNLOCKED, LOCKED)) {
                m_bOwnLock = true;
                return;
            }
        }

        hard_yield_cpu();
    }
}

bool SpinLock::TryLock() {
    if (m_bOwnLock) {
        throw new std::runtime_error("spin_lock_t is locked, cannot lock it again.");
    }

    return m_bOwnLock = atomic_cas(m_psl, UNLOCKED, LOCKED);
}

void SpinLock::Unlock() {
    if (!m_bOwnLock) {
        throw new std::runtime_error("spin_lock_t is unlocked, cannot unlock it again.");
    }
    atomic_zero(m_psl);
    m_bOwnLock = false;
}
} // namespace common
