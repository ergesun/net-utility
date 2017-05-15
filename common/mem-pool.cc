/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cassert>

#include "mem-pool.h"
#include "common-utils.h"

namespace netty {
    namespace common {
        MemPool::MemObject::~MemObject() {

        }

        void* MemPool::MemObject::Pointer() const {
            return reinterpret_cast<void*>(m_obj_pv);
        }

        uint32_t MemPool::MemObject::Size() const {
            return m_slot_size;
        }

        MemObjectType MemPool::MemObject::Type() const {
            return m_type;
        }

        uint32_t MemPool::MemObject::SlotSize() const {
            return m_slot_size;
        }

        uintptr_t MemPool::MemObject::ObjectPointerValue() const {
            return m_obj_pv;
        }

        uintptr_t MemPool::MemObject::SlotStartPointerValue() const {
            return m_slot_start_pv;
        }

        MemPool::MemPool() :
            MemPool(TINY_OBJECT_RESIDENT_CNT, ONE_SLOT_SMALL_OBJECT_RESIDENT_CNT, ONE_SLOT_BIG_OBJECT_RESIDENT_CNT,
                    EXTRA_OBJECT_CHECK_PERIOD, TINY_OBJECT_SIZE_THRESHOLD, (uint32_t)(BIG_PAGE_SIZE_THRESHOLD), BULK_PAGE_SIZE_THRESHOLD) {}

        MemPool::MemPool(uint32_t torc, uint32_t sorc, uint32_t borc, uint32_t eocp) :
            MemPool(torc, sorc, borc, eocp, TINY_OBJECT_SIZE_THRESHOLD, (uint32_t)(BIG_PAGE_SIZE_THRESHOLD), BULK_PAGE_SIZE_THRESHOLD) {}

        MemPool::MemPool(uint32_t torc, uint32_t sorc, uint32_t borc,
                         uint32_t eocp, uint32_t tpt, uint32_t bipt, uint32_t bupt) {
            m_sys_cacheline_size = (size_t)(common::CACHELINE_SIZE);
            m_sys_page_size = (size_t)(common::PAGE_SIZE);
            m_available_reserve_bulk_obj_max_size = RESIDENT_BULK_OBJ_MAX_SIZE;

            m_one_slot_tiny_obj_resident_cnts = torc;
            m_one_slot_small_obj_resident_cnts = sorc;
            m_small_obj_slot_footstep = DEFAULT_SMALL_OBJECT_SLOT_FOOTSTEP;
            m_one_slot_big_obj_resident_cnts = borc;
            m_recycle_extra_page_period = eocp;
            m_tiny_page_threshold = tpt;
            m_big_page_threshold = bipt;
            m_bult_page_threshold = bupt;

            m_recycle_check_cb = std::bind(&MemPool::check_objs, this);
            m_recycle_check_ev = Timer::Event(nullptr, &m_recycle_check_cb);

            m_recycle_check_timer = new Timer();
            m_recycle_check_timer->Start();
            m_recycle_check_timer->SubscribeEventAfter(uctime_t(m_recycle_extra_page_period, 0), m_recycle_check_ev);
        }

        MemPool::~MemPool() {

        }

        MemPool::MemObjectRef MemPool::Get(uint32_t size) {
            assert(size > 0);
            // TODO(sunchao): 减小锁的粒度
            MemObject *memObject = nullptr;
            if (m_tiny_page_threshold >= size) { // tiny obj operations.
                SpinLock l(&m_tiny_obj_pages_lock);
                uintptr_t tiny_obj_pv = 0;
                // 池子里没了，新申请。
                if (m_free_tiny_objs.empty()) {
                    auto objP = CommonUtils::PosixMemAlign(m_sys_page_size, m_sys_page_size);
                    if (objP) {
                        auto tiny_page_pv = reinterpret_cast<uintptr_t>(objP);
                        m_tiny_obj_pages.insert(tiny_page_pv);
                        auto splitObjs = split_mem_page(m_tiny_page_threshold, tiny_page_pv, m_tiny_page_threshold);
                        m_free_tiny_objs[tiny_page_pv] = splitObjs;
                    }
                }

                // 从池子中获取
                auto freeTinyObjsPageBegin = m_free_tiny_objs.begin();
                if (freeTinyObjsPageBegin != m_free_tiny_objs.end()) {
                    // 无需对freeTinyObjsBegin检查，如果外层判断存在那么内层一定存在，因为不存在的会删除不会留在free列表里。
                    auto freeTinyObjsBegin = freeTinyObjsPageBegin->second.begin();
                    tiny_obj_pv = *freeTinyObjsBegin;
                    freeTinyObjsPageBegin->second.erase(freeTinyObjsBegin);
                    if (freeTinyObjsPageBegin->second.empty()) {
                        m_free_tiny_objs.erase(freeTinyObjsPageBegin);
                    }
                }

                // 申请到了
                if (tiny_obj_pv) {
                    // 组装到MemObject中
                    if (!m_free_mem_objs.empty()) {
                        auto mo_begin = m_free_mem_objs.begin();
                        memObject = *mo_begin;
                        m_free_mem_objs.erase(mo_begin);
                    } else {
                        memObject = new MemObject(MemObjectType::Tiny, m_tiny_page_threshold, );
                    }
                }
            } else if (m_big_page_threshold >= size) { // small obj operations.

            } else if (m_bult_page_threshold >= size) { // big obj operations.

            } else { // bulk obj operations.

            }

            // 组装到MemObjectRef中
            return MemObjectRef(memObject);
        }

        void MemPool::Put(MemObjectRef mor) {

        }

        void MemPool::put(int32_t slot_size, uintptr_t slot_start_p, uintptr_t release_p) {

        }

        void MemPool::check_objs() {
            m_recycle_check_timer->SubscribeEventAfter(uctime_t(m_recycle_extra_page_period, 0), m_recycle_check_ev);
        }

        std::list<uintptr_t> MemPool::split_mem_page(uint32_t slotSize, uintptr_t pagePv, uint32_t pageSize) {
            std::list<uintptr_t> res;
            uint32_t cnt = pageSize / slotSize;
            for (int i = 0; i < cnt; ++i) {
                res.push_back(pagePv + slotSize);
            }

            return res;
        }
    }  // namespace common
}  // namespace netty
