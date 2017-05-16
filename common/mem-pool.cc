/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cassert>
#include <cmath>

#include "mem-pool.h"
#include "common-utils.h"

namespace netty {
    namespace common {
        template <typename T>
        T* MemPool::MemObject::Pointer() const
        {
            return reinterpret_cast<T*>(m_obj_pv);
        }

        uint32_t MemPool::MemObject::Size() const {
            return m_slot_size;
        }

        MemPool::MemObjectType MemPool::MemObject::Type() const {
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
            m_sys_cacheline_size = (uint32_t)(common::CACHELINE_SIZE);
            m_sys_page_size = (uint32_t)(common::PAGE_SIZE);
            m_available_reserve_bulk_obj_max_size = RESIDENT_BULK_OBJ_MAX_SIZE;

            m_one_slot_tiny_obj_resident_cnts = torc;
            m_one_slot_small_obj_resident_cnts = sorc;
            m_small_obj_slot_footstep_size = DEFAULT_SMALL_OBJECT_SLOT_FOOTSTEP;
            m_one_slot_big_obj_resident_cnts = borc;
            m_recycle_extra_page_period = eocp;
            m_tiny_obj_threshold = tpt;
            m_big_obj_threshold = bipt;
            m_bulk_obj_threshold = bupt;
            m_expand_obj_cnt_factor = DEFAULT_EXPAND_OBJ_CNT_FACTOR;

            // 分配小对象的槽
            auto smallSlotCnt = m_big_obj_threshold / m_small_obj_slot_footstep_size;
            for (int i = 1; i <= smallSlotCnt; ++i) {
                m_free_small_objs[i] = SlotObjPage();
                m_small_obj_pages[i] = std::unordered_set<uintptr_t>();
            }

            // 分配大对象的槽
            auto bigSlotCnt = m_bulk_obj_threshold / m_sys_page_size;
            for (int i = 1; i <= bigSlotCnt; ++i) {
                m_free_big_objs[i] = SlotObjPage();
                m_big_obj_pages[i] = std::unordered_set<uintptr_t>();
            }

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
            MemObject *memObject = nullptr;
            if (m_tiny_obj_threshold >= size) { // tiny obj operations.
                SpinLock l(&m_tiny_obj_pages_lock);

                // 池子里没了，新申请。
                if (m_free_tiny_objs.empty()) {
                    alloc_page_objs(m_sys_page_size, m_tiny_obj_threshold, m_tiny_obj_pages, m_free_tiny_objs);
                }

                // 从池子中获取
                uintptr_t tiny_obj_pv = 0;
                uintptr_t slot_page_pv = 0;
                get_free_obj_from_slot_page(m_free_tiny_objs, slot_page_pv, tiny_obj_pv);

                // 申请到了
                if (tiny_obj_pv) {
                    // 组装到MemObject中
                    memObject = get_mem_object(MemObjectType::Tiny, m_tiny_obj_threshold, tiny_obj_pv, slot_page_pv);
                }
            } else if (m_bulk_obj_threshold >= size) { // slot obj(big、small的模式一样) operations.
                uint32_t footstepSize = 0;
                std::unordered_map<uint32_t, SlotObjPage> *free_objs;
                std::unordered_map<uint32_t, std::unordered_set<uintptr_t>> *pages;
                MemObjectType type;
                if (m_big_obj_threshold < size) { // big obj
                    footstepSize = m_sys_page_size;
                    free_objs = &m_free_big_objs;
                    pages = &m_big_obj_pages;
                    type = MemObjectType::Big;
                } else { // small obj
                    footstepSize = m_small_obj_slot_footstep_size;
                    free_objs = &m_free_small_objs;
                    pages = &m_small_obj_pages;
                    type = MemObjectType::Small;
                }

                auto slotFootstep = (uint32_t)log2(footstepSize);
                auto slotSize = RoundupToTheNextHighestMultipleOfExponent2(size, slotFootstep);
                auto slotIdx = convert_slot_obj_size_to_slot_idx(slotSize);

                auto slotPageIter = free_objs->find(slotIdx);
                // 对相应槽位加锁
                SpinLock l(&slotPageIter->second.sl);

                // 无可用对象
                if (slotPageIter->second.objs.empty()) {
                    auto expandPageSize = gen_expand_slot_page_size(slotSize, m_expand_obj_cnt_factor, m_sys_page_size);
                    alloc_page_objs(expandPageSize, slotSize, (*pages)[slotIdx], (*free_objs)[slotIdx].objs);
                }

                uintptr_t slot_page_pv = 0;
                uintptr_t obj_pv = 0;
                get_free_obj_from_slot_page((*free_objs)[slotIdx].objs, slot_page_pv, obj_pv);
                // 申请到了
                if (obj_pv) {
                    // 组装到MemObject中
                    memObject = get_mem_object(type, slotSize, obj_pv, slot_page_pv);
                }
            } else { // bulk obj operations.
                SpinLock l(&m_bulk_obj_pages_lock);
                uintptr_t suitableObjPv;
                uint32_t pageSize;
                get_suitable_bulk_page(size, suitableObjPv, pageSize);
                if (!suitableObjPv) {
                    auto bulkPageFootstep = (uint32_t)log2(m_sys_page_size);
                    pageSize = RoundupToTheNextHighestMultipleOfExponent2(size, bulkPageFootstep);
                    auto pPage = CommonUtils::PosixMemAlign(m_sys_page_size, size);
                    if (pPage) {

                    }
                }

                if (suitableObjPv) {
                    memObject = get_mem_object(MemObjectType::Bulk, pageSize, suitableObjPv, suitableObjPv);
                }
            }

            // 组装到MemObjectRef中
            return MemObjectRef(memObject);
        }

        void MemPool::Put(MemObjectRef mor) {
            auto memObject = mor.get();
            put(memObject->Type(), memObject->SlotSize(),
                memObject->ObjectPointerValue(), memObject->SlotStartPointerValue());
            m_free_mem_objs.push_back(memObject);
        }

        void MemPool::put(MemObjectType type, int32_t slot_size, uintptr_t obj_pv, uintptr_t slot_start_pv) {

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

        MemPool::MemObject * MemPool::get_mem_object(MemObjectType type, uint32_t slotSize, uintptr_t objPv,
                                                     uintptr_t slotStartPv) {
            MemObject *memObject = nullptr;
            if (!m_free_mem_objs.empty()) {
                auto mo_begin = m_free_mem_objs.begin();
                memObject = *mo_begin;
                memObject->refresh(type, slotSize, objPv, slotStartPv);
                m_free_mem_objs.erase(mo_begin);
            } else {
                memObject = new MemObject(type, slotSize, objPv, slotStartPv);
            }

            return memObject;
        }

        void MemPool::MemObject::refresh(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t slotStartPv) {
            m_type = type;
            m_slot_size = slotSize;
            m_obj_pv = objPv;
            m_slot_start_pv = slotStartPv;
        }

        void MemPool::alloc_page_objs(uint32_t size, uint32_t slotSize, std::unordered_set<uintptr_t> &pages,
                                      std::unordered_map<uintptr_t, std::list<uintptr_t>> &freeObjs) {
            auto objP = CommonUtils::PosixMemAlign(m_sys_page_size, size);
            if (objP) {
                auto page_pv = reinterpret_cast<uintptr_t>(objP);
                pages.insert(page_pv);
                auto splitObjs = split_mem_page(slotSize, page_pv, size);
                freeObjs[page_pv] = splitObjs;
            }
        }

        void MemPool::get_free_obj_from_slot_page(std::unordered_map<uintptr_t, std::list<uintptr_t>> &pages,
                                                  uintptr_t &slot_page_pv, uintptr_t &obj_pv) {
            slot_page_pv = 0;
            auto freeObjsPageBegin = pages.begin();
            if (freeObjsPageBegin != pages.end()) {
                slot_page_pv = freeObjsPageBegin->first;
                // 无需对freeTinyObjsBegin检查，如果外层判断存在那么内层一定存在，因为不存在的会删除不会留在free列表里。
                auto freeObjsBegin = freeObjsPageBegin->second.begin();
                obj_pv = *freeObjsBegin;
                freeObjsPageBegin->second.erase(freeObjsBegin);
                if (freeObjsPageBegin->second.empty()) {
                    pages.erase(freeObjsPageBegin);
                }
            }
        }

        void MemPool::get_suitable_bulk_page(uint32_t size, uintptr_t &pagePv, uint32_t &pageSize, float moreThanFactor) {
            pagePv = 0;
            uint32_t maxPageSize = (uint32_t)(size * moreThanFactor);
            if (m_free_bulk_objs.empty()) {
                return;
            }

            auto rbegin = m_free_bulk_objs.rbegin();
            if (rbegin->first < size) {
                return;
            }

            std::map<uint32_t, std::list<uintptr_t>>::iterator *suitableIter = nullptr;
            auto slotBegin = m_free_bulk_objs.begin();
            if (slotBegin->first > size) {
                if (slotBegin->first > maxPageSize) {
                    return;
                }

                pageSize = slotBegin->first;
                suitableIter = &slotBegin;
            }

            if (!suitableIter) {
                std::map<uint32_t, std::list<uintptr_t>>::reverse_iterator *suitableRIter = nullptr;
                for (; rbegin != m_free_bulk_objs.rend(); ++rbegin) {
                    if (rbegin->first > size && rbegin->first <= maxPageSize) {
                        pageSize = rbegin->first;
                        suitableRIter = &rbegin;
                        break;
                    }
                }

                if (suitableRIter) {
                    auto suitableObjBegin = (*suitableRIter)->second.begin();
                    pagePv = *suitableObjBegin;
                    (*suitableRIter)->second.erase(suitableObjBegin);
                    if ((*suitableRIter)->second.empty()) {
                        m_free_bulk_objs.erase((*suitableRIter)->first);
                    }
                }
            } else {
                auto suitableObjBegin = (*suitableIter)->second.begin();
                pagePv = *suitableObjBegin;
                (*suitableIter)->second.erase(suitableObjBegin);
                if ((*suitableIter)->second.empty()) {
                    m_free_bulk_objs.erase(*suitableIter);
                }
            }
        }
    }  // namespace common
}  // namespace netty
