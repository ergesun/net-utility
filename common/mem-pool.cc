/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cassert>

#include "mem-pool.h"

namespace netty {
    namespace common {
        MemPool::MemObject::MemObject() {

        }

        MemPool::MemObject::~MemObject() {

        }

        void* MemPool::MemObject::GetPointer() const {

        }

        int32_t MemPool::MemObject::GetSize() const {

        }

        MemPool::MemPool() {
            m_cacheline_size = CACHELINE_SIZE;
            m_small_obj_resident_cnts = SMALL_OBJECT_RESIDENT_CNT;
            m_big_obj_resident_cnts = BIG_OBJECT_RESIDENT_CNT;
            m_bult_page_threshold = BULK_PAGE_SIZE_THRESHOLD;
            m_recycle_extra_page_period = EXTRA_OBJECT_CHECK_PERIOD;

            m_recycle_check_cb = std::bind(&MemPool::check_recycle, this);
            m_recycle_check_ev = Timer::Event(nullptr, &m_recycle_check_cb);

            m_recycle_check_timer = new Timer();
            m_recycle_check_timer->Start();
            m_recycle_check_timer->SubscribeEventAfter(uctime_t(m_recycle_extra_page_period, 0), m_recycle_check_ev);
        }

        MemPool::MemPool(uint32_t sorc, uint32_t borc, uint32_t eocp) {
            m_cacheline_size = CACHELINE_SIZE;
            m_small_obj_resident_cnts = sorc;
            m_big_obj_resident_cnts = borc;
            m_recycle_extra_page_period = eocp;

            m_bult_page_threshold = BULK_PAGE_SIZE_THRESHOLD;

            m_recycle_check_cb = std::bind(&MemPool::check_recycle, this);
            m_recycle_check_ev = Timer::Event(nullptr, &m_recycle_check_cb);

            m_recycle_check_timer = new Timer();
            m_recycle_check_timer->Start();
            m_recycle_check_timer->SubscribeEventAfter(uctime_t(m_recycle_extra_page_period, 0), m_recycle_check_ev);
        }

        MemPool::~MemPool() {

        }

        MemPool::MemObjectRef MemPool::Get(uint32_t size) {
            assert(size > 0);

        }

        void MemPool::put(int32_t slot_size, uintptr_t slot_start_p, uintptr_t release_p) {

        }

        void MemPool::check_recycle() {
            m_recycle_check_timer->SubscribeEventAfter(uctime_t(m_recycle_extra_page_period, 0), m_recycle_check_ev);
        }
    }  // namespace common
}  // namespace netty
