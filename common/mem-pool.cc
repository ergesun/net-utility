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
        char* MemPool::MemObject::Pointer() const {
            return reinterpret_cast<char*>(m_obj_pv);
        }

        uint32_t MemPool::MemObject::Size() const {
            return m_slot_size;
        }

        void MemPool::MemObject::Put() {
            m_ownnerPool->Put(this);
        }

        MemPool::MemObjectType MemPool::MemObject::Type() const {
            return m_type;
        }

        uint32_t MemPool::MemObject::SlotIdx() const {
            return m_slot_size;
        }

        uintptr_t MemPool::MemObject::ObjectPointerValue() const {
            return m_obj_pv;
        }

        uintptr_t MemPool::MemObject::ObjectPagePointerValue() const {
            return m_obj_page_pv;
        }

        MemPool::MemPool() :
            MemPool(TINY_OBJECT_RESIDENT_CNT, ONE_SLOT_SMALL_OBJECT_RESIDENT_CNT, ONE_SLOT_BIG_OBJECT_RESIDENT_CNT,
                    TINY_OBJECT_SIZE_THRESHOLD, (uint32_t)(BIG_PAGE_SIZE_THRESHOLD), BULK_PAGE_SIZE_THRESHOLD) {}

        MemPool::MemPool(uint32_t torc, uint32_t sorc, uint32_t borc) :
            MemPool(torc, sorc, borc, TINY_OBJECT_SIZE_THRESHOLD, (uint32_t)(BIG_PAGE_SIZE_THRESHOLD), BULK_PAGE_SIZE_THRESHOLD) {}

        MemPool::MemPool(uint32_t torc, uint32_t sorc, uint32_t borc,
                         uint32_t tpt, uint32_t bipt, uint32_t bupt) {
            m_sys_page_size = (uint32_t)(common::PAGE_SIZE);
            m_available_reserve_bulk_obj_max_size = RESIDENT_BULK_OBJ_MAX_SIZE;

            m_one_slot_tiny_obj_resident_cnts = torc;
            m_one_slot_small_obj_resident_cnts = sorc;
            m_small_obj_slot_footstep_size = DEFAULT_SMALL_OBJECT_SLOT_FOOTSTEP;
            m_small_obj_slot_footstep_exponent = (uint32_t)log2(m_small_obj_slot_footstep_size);
            m_big_obj_slot_footstep_exponent = (uint32_t)log2(m_sys_page_size);
            m_bulk_obj_slot_footstep_exponent = (uint32_t)log2(m_sys_page_size);
            m_one_slot_big_obj_resident_cnts = borc;
            m_one_slot_bulk_obj_resident_cnts = (uint32_t)(ONE_SLOT_BULK_OBJECT_RESIDENT_CNT);
            m_tiny_obj_threshold = tpt;
            m_big_obj_threshold = bipt;
            m_bulk_obj_threshold = bupt;
            m_expand_obj_cnt_factor = DEFAULT_EXPAND_OBJ_CNT_FACTOR;
            m_expand_bulk_obj_cnt_factor = DEFAULT_EXPAND_BULK_OBJ_CNT_FACTOR;

            // 分配小对象的槽
            auto smallSlotCnt = m_big_obj_threshold / m_small_obj_slot_footstep_size;
            m_free_small_objs.reserve(smallSlotCnt);
            m_small_obj_pages.reserve(smallSlotCnt);
            for (uint32_t i = 1; i <= smallSlotCnt; ++i) {
                m_free_small_objs[i] = SlotObjPage();
                m_small_obj_pages[i] = std::unordered_set<uintptr_t>();
            }

            // 分配大对象的槽
            auto bigSlotCnt = m_bulk_obj_threshold / m_sys_page_size;
            m_free_big_objs.reserve(bigSlotCnt);
            m_big_obj_pages.reserve(bigSlotCnt);
            for (uint32_t i = 1; i <= bigSlotCnt; ++i) {
                m_free_big_objs[i] = SlotObjPage();
                m_big_obj_pages[i] = std::unordered_set<uintptr_t>();
            }
        }

        MemPool::~MemPool() {
            /*tiny*/
            for (auto iter = m_tiny_obj_pages.begin(); iter != m_tiny_obj_pages.end(); ++iter) {
                free(reinterpret_cast<char*>(*iter));
            }

            /*small*/
            for (auto iter = m_small_obj_pages.begin(); iter != m_small_obj_pages.end(); ++iter) {
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    free(reinterpret_cast<char*>(*iter2));
                }
            }

            /*big*/
            for (auto iter = m_big_obj_pages.begin(); iter != m_big_obj_pages.end(); ++iter) {
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    free(reinterpret_cast<char*>(*iter2));
                }
            }

            /*bulk*/
            for (auto iter = m_bulk_obj_pages.begin(); iter != m_bulk_obj_pages.end(); ++iter) {
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    free(reinterpret_cast<char*>(*iter2));
                }
            }

            for (auto iter = m_free_mem_objs.begin(); iter != m_free_mem_objs.end(); ++iter) {
                delete *iter;
            }
        }

        MemPool::MemObject* MemPool::Get(uint32_t size) {
            assert(size > 0);
            MemObject *memObject = nullptr;
            if (m_tiny_obj_threshold >= size) { // tiny obj operations.
                SpinLock l(&m_tiny_obj_pages_lock);

                // 池子里没了，新申请。
                if (m_free_tiny_objs.empty()) {
                    if (alloc_page_objs(m_sys_page_size, m_tiny_obj_threshold, m_tiny_obj_pages, m_free_tiny_objs)) {
                        m_free_tiny_objs_cnt = m_sys_page_size / m_tiny_obj_threshold;
                    }
                }

                // 从池子中获取
                uintptr_t tiny_obj_pv = 0;
                uintptr_t slot_page_pv = 0;
                get_free_obj_from_slot_page(m_free_tiny_objs, slot_page_pv, tiny_obj_pv);

                // 申请到了
                if (tiny_obj_pv) {
                    --m_free_tiny_objs_cnt;
                    // 组装到MemObject中
                    memObject = get_mem_object(MemObjectType::Tiny, m_tiny_obj_threshold, tiny_obj_pv, slot_page_pv);
                }
            } else if (m_bulk_obj_threshold >= size) { // slot obj(big、small的模式一样) operations.
                uint32_t slotFootstepSize = 0;
                uint32_t slotFootstepExponent = 0;
                std::unordered_map<uint32_t, SlotObjPage> *free_objs;
                std::unordered_map<uint32_t, std::unordered_set<uintptr_t>> *pages;
                MemObjectType type;
                if (m_big_obj_threshold < size) { // big obj
                    slotFootstepSize = m_sys_page_size;
                    slotFootstepExponent = m_big_obj_slot_footstep_exponent;
                    free_objs = &m_free_big_objs;
                    pages = &m_big_obj_pages;
                    type = MemObjectType::Big;
                } else { // small obj
                    slotFootstepSize = m_small_obj_slot_footstep_size;
                    slotFootstepExponent = m_small_obj_slot_footstep_exponent;
                    free_objs = &m_free_small_objs;
                    pages = &m_small_obj_pages;
                    type = MemObjectType::Small;
                }

                auto slotSize = roundup_to_the_next_highest_multiple_of_exponent2(size, slotFootstepExponent);
                auto slotIdx = convert_slot_obj_size_to_slot_idx(slotSize, slotFootstepSize);

                auto slotPageIter = free_objs->find(slotIdx);
                // 对相应槽位加锁
                SpinLock l(&slotPageIter->second.sl);

                // 无可用对象
                if (slotPageIter->second.objs.empty()) {
                    auto expandPageSize = gen_expand_slot_page_size(slotSize, m_expand_obj_cnt_factor, m_sys_page_size);
                    if (alloc_page_objs(expandPageSize, slotSize, (*pages)[slotIdx], (*free_objs)[slotIdx].objs)) {
                        (*free_objs)[slotIdx].cnt = expandPageSize / slotSize;
                    }
                }

                uintptr_t slot_page_pv = 0;
                uintptr_t obj_pv = 0;
                get_free_obj_from_slot_page((*free_objs)[slotIdx].objs, slot_page_pv, obj_pv);
                // 申请到了
                if (obj_pv) {
                    --((*free_objs)[slotIdx].cnt);
                    // 组装到MemObject中
                    memObject = get_mem_object(type, slotSize, obj_pv, slot_page_pv);
                }
            } else { // bulk obj operations.
                SpinLock l(&m_bulk_obj_pages_lock);
                uint32_t needPageSize = roundup_to_the_next_highest_multiple_of_exponent2(size, m_bulk_obj_slot_footstep_exponent);
                uint32_t needSlotIdx = convert_slot_obj_size_to_slot_idx(needPageSize, m_sys_page_size);
                uintptr_t suitableObjPv, suitablePagePv;
                uint32_t suitableObjSlotIdx;
                get_suitable_bulk_page(needSlotIdx, suitableObjSlotIdx, suitableObjPv, suitablePagePv);
                if (!suitableObjPv) {
                    uint32_t expandPageSize = needPageSize;
                    if (needPageSize < m_available_reserve_bulk_obj_max_size) {
                        // 只有可以reserve的对象才会预分配，否则只分配一个，put时直接丢弃。
                        expandPageSize = gen_expand_slot_page_size(needPageSize, m_expand_bulk_obj_cnt_factor, m_sys_page_size);
                    }

                    m_bulk_obj_pages[needSlotIdx] = std::unordered_set<uintptr_t>();
                    m_free_hash_bulk_objs[needSlotIdx] = SlotObjPage();
                    m_free_bulk_objs[needSlotIdx] = &(m_free_hash_bulk_objs[needSlotIdx]);
                    if (alloc_page_objs(expandPageSize, needPageSize, m_bulk_obj_pages[needSlotIdx], m_free_hash_bulk_objs[needSlotIdx].objs)) {
                        m_free_hash_bulk_objs[needSlotIdx].cnt = expandPageSize / needPageSize;
                    } else {
                        m_free_hash_bulk_objs.erase(needSlotIdx);
                        m_free_bulk_objs.erase(needSlotIdx);
                    }

                    get_suitable_bulk_page(needSlotIdx, suitableObjSlotIdx, suitableObjPv, suitablePagePv);
                }

                if (suitableObjPv) {
                    // O(1)
                    if (m_free_hash_bulk_objs.end() != m_free_hash_bulk_objs.find(suitableObjSlotIdx)) {
                        --(m_free_hash_bulk_objs[suitableObjSlotIdx].cnt);
                    }

                    memObject = get_mem_object(MemObjectType::Bulk, suitableObjSlotIdx * m_sys_page_size, suitableObjPv, suitablePagePv);
                }
            }

            // 组装到MemObjectRef中
            if (!memObject) {
                fprintf(stderr, "fail to alloc mem object.\n");
            }
            return memObject;
        }

        void MemPool::Put(MemObject *memObject) {
            if (!memObject) {
                return;
            }

            put(memObject->Type(), memObject->SlotIdx(),
                memObject->ObjectPointerValue(), memObject->ObjectPagePointerValue());
            SpinLock l(&m_free_mem_objs_lock);
            m_free_mem_objs.push_back(memObject);
        }

        /**
         * 懒得抽成函数，复制粘贴挺好，清晰明了。以后万一不同level的对象有不同策略也不用拆了。
         * @param type
         * @param slotSize
         * @param objPv
         * @param objPagePv
         */
        void MemPool::put(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t objPagePv) {
            switch (type) {
                case MemObjectType::Tiny: {
                    SpinLock l(&m_tiny_obj_pages_lock);
                    auto objIter = m_free_tiny_objs.find(objPagePv);
                    if (objIter == m_free_tiny_objs.end()) {
                        m_free_tiny_objs[objPagePv] = std::list<uintptr_t>();
                    }

                    m_free_tiny_objs[objPagePv].push_back(objPv);
                    auto onePageObjsCnt = m_sys_page_size / m_tiny_obj_threshold;
                    if (++m_free_tiny_objs_cnt >= m_one_slot_tiny_obj_resident_cnts + onePageObjsCnt) {
                        // free的obj个数超出了预设的个数，需要找一个满闲page释放
                        for (auto iter = m_free_tiny_objs.begin(); iter != m_free_tiny_objs.end(); ++iter) {
                            if (onePageObjsCnt == iter->second.size()) {
                                // 找到了满闲的page，移除之
                                auto freePtr = iter->first;
                                m_tiny_obj_pages.erase(freePtr);
                                m_free_tiny_objs.erase(iter);
                                m_free_tiny_objs_cnt -= onePageObjsCnt;
                                free(reinterpret_cast<char*>(freePtr));
                                break;
                            }
                        }
                    }
                    break;
                }
                case MemObjectType::Small: {
                    auto slotIdx = convert_slot_obj_size_to_slot_idx(slotSize, m_small_obj_slot_footstep_size);
                    auto slotIter = m_free_small_objs.find(slotIdx);
                    SpinLock l(&slotIter->second.sl);
                    auto pagePvIter = slotIter->second.objs.find(objPagePv);
                    if (slotIter->second.objs.end() == pagePvIter) {
                        slotIter->second.objs[objPagePv] = std::list<uintptr_t>();
                    }

                    slotIter->second.objs[objPagePv].push_back(objPv);
                    auto onePageSize = gen_expand_slot_page_size(slotSize, m_expand_obj_cnt_factor, m_sys_page_size);
                    auto onePageObjsCnt = onePageSize / slotSize;
                    if (++(slotIter->second.cnt) >= m_one_slot_small_obj_resident_cnts + onePageObjsCnt) {
                        // free的obj个数超出了预设的个数，需要找一个满闲page释放
                        for (auto iter = slotIter->second.objs.begin(); iter != slotIter->second.objs.end(); ++iter) {
                            // 找到了满闲的page，移除之
                            if (onePageObjsCnt == iter->second.size()) {
                                auto freePtr = iter->first;
                                m_small_obj_pages[slotIdx].erase(freePtr);
                                slotIter->second.objs.erase(iter);
                                slotIter->second.cnt -= onePageObjsCnt;
                                free(reinterpret_cast<char*>(freePtr));
                                break;
                            }
                        }
                    }
                    break;
                }
                case MemObjectType::Big: {
                    auto slotIdx = convert_slot_obj_size_to_slot_idx(slotSize, m_sys_page_size);

                    auto slotIter = m_free_big_objs.find(slotIdx);
                    SpinLock l(&slotIter->second.sl);
                    auto pagePvIter = slotIter->second.objs.find(objPagePv);
                    if (slotIter->second.objs.end() == pagePvIter) {
                        slotIter->second.objs[objPagePv] = std::list<uintptr_t>();
                    }

                    slotIter->second.objs[objPagePv].push_back(objPv);
                    auto onePageSize = gen_expand_slot_page_size(slotSize, m_expand_obj_cnt_factor, m_sys_page_size);
                    auto onePageObjsCnt = onePageSize / slotSize;
                    if (++(slotIter->second.cnt) >= m_one_slot_big_obj_resident_cnts + onePageObjsCnt) {
                        for (auto iter = slotIter->second.objs.begin(); iter != slotIter->second.objs.end(); ++iter) {
                            if (onePageObjsCnt == iter->second.size()) {
                                auto freePtr = iter->first;
                                m_big_obj_pages[slotIdx].erase(freePtr);
                                slotIter->second.objs.erase(iter);
                                slotIter->second.cnt -= onePageObjsCnt;
                                free(reinterpret_cast<char*>(freePtr));
                                break;
                            }
                        }
                    }
                    break;
                }
                case MemObjectType::Bulk: {
                    SpinLock l(&m_bulk_obj_pages_lock);
                    auto slotIdx = convert_slot_obj_size_to_slot_idx(slotSize, m_sys_page_size);
                    if (slotSize > m_available_reserve_bulk_obj_max_size) {
                        auto dropObjSlotIter = m_bulk_obj_pages.find(slotIdx);
                        dropObjSlotIter->second.erase(objPagePv);
                        free(reinterpret_cast<char*>(objPagePv));
                        if (dropObjSlotIter->second.empty()) {
                            m_bulk_obj_pages.erase(slotIdx);
                        }

                        return;
                    }

                    auto slotIter = m_free_hash_bulk_objs.find(slotIdx);
                    if (m_free_hash_bulk_objs.end() == slotIter) {
                        m_free_hash_bulk_objs[slotIdx] = SlotObjPage();
                        m_free_hash_bulk_objs[slotIdx].objs[objPagePv] = std::list<uintptr_t>();
                        m_free_hash_bulk_objs[slotIdx].cnt = 0;
                        m_free_bulk_objs[slotIdx] = &(m_free_hash_bulk_objs[slotIdx]);
                    }

                    m_free_hash_bulk_objs[slotIdx].objs[objPagePv].push_back(objPv);
                    auto onePageSize = gen_expand_slot_page_size(slotSize, m_expand_bulk_obj_cnt_factor, m_sys_page_size);
                    auto onePageObjsCnt = onePageSize / slotSize;
                    if (++(m_free_hash_bulk_objs[slotIdx].cnt) >= m_one_slot_bulk_obj_resident_cnts + onePageObjsCnt) {
                        for (auto iter = m_free_hash_bulk_objs[slotIdx].objs.begin(); iter != m_free_hash_bulk_objs[slotIdx].objs.end(); ++iter) {
                            if (onePageObjsCnt == iter->second.size()) {
                                //free(reinterpret_cast<char*>(iter->first));
                                auto freePtr = iter->first;
                                m_bulk_obj_pages[slotIdx].erase(freePtr);
                                m_free_hash_bulk_objs[slotIdx].objs.erase(iter);
                                free(reinterpret_cast<char*>(freePtr));
                                m_free_hash_bulk_objs[slotIdx].cnt -= onePageObjsCnt;
                                if (0 == m_free_hash_bulk_objs[slotIdx].cnt) {
                                    m_free_hash_bulk_objs.erase(slotIdx);
                                    m_free_bulk_objs.erase(slotIdx);
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

        std::list<uintptr_t> MemPool::split_mem_page(uint32_t slotSize, uintptr_t pagePv, uint32_t pageSize) {
            std::list<uintptr_t> res;
            uint32_t cnt = pageSize / slotSize;
            for (uint32_t i = 0; i < cnt; ++i) {
                res.push_back(pagePv + slotSize * i);
            }

            return res;
        }

        MemPool::MemObject * MemPool::get_mem_object(MemObjectType type, uint32_t slotSize, uintptr_t objPv,
                                                     uintptr_t slotStartPv) {
            MemObject *memObject = nullptr;
            SpinLock l(&m_free_mem_objs_lock);
            if (!m_free_mem_objs.empty()) {
                auto mo_begin = m_free_mem_objs.begin();
                memObject = *mo_begin;
                memObject->refresh(type, slotSize, objPv, slotStartPv);
                m_free_mem_objs.erase(mo_begin);
            } else {
                memObject = new MemObject(type, slotSize, objPv, slotStartPv, this);
            }

            return memObject;
        }

        void MemPool::MemObject::refresh(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t objPagePv) {
            m_type = type;
            m_slot_size = slotSize;
            m_obj_pv = objPv;
            m_obj_page_pv = objPagePv;
        }

        bool MemPool::alloc_page_objs(uint32_t size, uint32_t slotSize, std::unordered_set<uintptr_t> &pages,
                                      std::unordered_map<uintptr_t, std::list<uintptr_t>> &freeObjs) {
            auto objP = CommonUtils::PosixMemAlign(m_sys_page_size, size);
            if (objP) {
                auto page_pv = reinterpret_cast<uintptr_t>(objP);
                pages.insert(page_pv);
                auto splitObjs = split_mem_page(slotSize, page_pv, size);
                freeObjs[page_pv] = splitObjs;

                return true;
            }

            return false;
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

        void MemPool::get_suitable_bulk_page(uint32_t needSlotIdx, uint32_t &suitableSlotIdx, uintptr_t &suitableObjPv,
                                             uintptr_t &suitablePagePv, float moreThanFactor) {
            suitableSlotIdx = 0;
            suitableObjPv = 0;
            suitablePagePv = 0;
            auto okHashIter = m_free_hash_bulk_objs.find(needSlotIdx);
            // 如果直接找到了完全匹配的，直接返回 O(1)。
            if (okHashIter != m_free_hash_bulk_objs.end()) {
                auto okObjIter = okHashIter->second.objs.begin();
                auto okObjPvIter = okObjIter->second.begin();
                suitableSlotIdx = okHashIter->first;
                suitableObjPv = *okObjPvIter;
                suitablePagePv = okObjIter->first;

                okObjIter->second.erase(okObjPvIter);
                if (okObjIter->second.empty()) {
                    // 从free列表移除
                    okHashIter->second.objs.erase(okObjIter);
                    if (okHashIter->second.objs.empty()) {
                        m_free_hash_bulk_objs.erase(needSlotIdx);
                        m_free_bulk_objs.erase(needSlotIdx);
                    }
                }

                return;
            }

            /**
             * 如果直接没找到，执行下面的逻辑。
             */
            // 如果没有free的，直接返回。
            if (m_free_bulk_objs.empty()) {
                return;
            }

            uint32_t maxPageSize = (uint32_t)(needSlotIdx * moreThanFactor);

            auto rbegin = m_free_bulk_objs.rbegin();
            // 如果最大的(最后一个)都比要求的小，直接返回。
            if (rbegin->first < needSlotIdx) {
                return;
            }


            std::map<uint32_t, SlotObjPage*>::iterator *suitableIter = nullptr;
            auto slotBegin = m_free_bulk_objs.begin();
            if (slotBegin->first > needSlotIdx) {
                // 如果最小的(第一个)都比要求的大的范围不满足，直接返回。
                if (slotBegin->first > maxPageSize) {
                    return;
                }

                suitableSlotIdx = slotBegin->first;
                suitableIter = &slotBegin;
            }

            if (!suitableIter) { // 如果空说明最小的不匹配，那么就开始从最大的往前找。
                // TODO(sunchao): 优化此查找算法，比如二分查找？感觉没太大必要，毕竟bulk page很少。
                std::map<uint32_t, SlotObjPage*>::reverse_iterator *suitableRIter = nullptr;
                for (; rbegin != m_free_bulk_objs.rend(); ++rbegin) {
                    // 找到了合适的
                    if (rbegin->first > needSlotIdx && rbegin->first <= maxPageSize) {
                        suitableSlotIdx = rbegin->first;
                        suitableRIter = &rbegin;
                        break;
                    }
                }

                if (suitableRIter) { // 如果有合适的
                    auto suitableObjPageBegin = (*suitableRIter)->second->objs.begin();
                    auto suitableObjBegin = suitableObjPageBegin->second.begin();
                    suitableObjPv = *suitableObjBegin;
                    suitablePagePv = suitableObjPageBegin->first;
                    (*suitableRIter)->second->objs[suitablePagePv].erase(suitableObjBegin);
                    if ((*suitableRIter)->second->objs[suitablePagePv].empty()) {
                        (*suitableRIter)->second->objs.erase(suitableObjPageBegin);
                        if ((*suitableRIter)->second->objs.empty()) {
                            m_free_bulk_objs.erase((*suitableRIter)->first);
                            m_free_hash_bulk_objs.erase((*suitableRIter)->first);
                        }
                    }
                }
            } else { // 最小的满足
                auto suitableObjPageBegin = (*suitableIter)->second->objs.begin();
                auto suitableObjBegin = suitableObjPageBegin->second.begin();
                suitableObjPv = *suitableObjBegin;
                suitablePagePv = suitableObjPageBegin->first;
                (*suitableIter)->second->objs[suitablePagePv].erase(suitableObjBegin);
                if ((*suitableIter)->second->objs[suitablePagePv].empty()) {
                    (*suitableIter)->second->objs.erase(suitableObjPageBegin);
                    if ((*suitableIter)->second->objs.empty()) {
                        m_free_bulk_objs.erase((*suitableIter)->first);
                        m_free_hash_bulk_objs.erase((*suitableIter)->first);
                    }
                }
            }
        }

        std::string MemPool::DumpDebugInfo() {
#define ALIGN2    "  "
#define ALIGN4    "    "
#define ALIGN6    "      "
#define ALIGN8    "        "
#define SEPERATOR "-------------------------------------------------------------------"

            std::stringstream ss;
            /****************tiny**********************/
            ss << "[Tiny]\n"
               << SEPERATOR << "\n"
               << ALIGN2 << "[[All pages]]:\n"
               << ALIGN4 << "count: " << m_tiny_obj_pages.size() << ";\n"
               << ALIGN4 << "pages: ";
            std::stringstream tinyPagesSS;
            for (auto iter = m_tiny_obj_pages.begin(); iter != m_tiny_obj_pages.end(); ++iter) {
                tinyPagesSS << *iter << ",";
            }

            auto tinyPagesStr = tinyPagesSS.str();
            ss << tinyPagesStr.substr(0, tinyPagesStr.length() - 1) << "\n";

            ss << ALIGN2 << "[[Free Objs]]:\n"
               << ALIGN4 << "count: " << m_free_tiny_objs_cnt << ";\n"
               << ALIGN4 << "objects: \n" << ALIGN6;
            std::stringstream tinyTotalObjsSS;
            for (auto iter = m_free_tiny_objs.begin(); iter != m_free_tiny_objs.end(); ++iter) {
                std::stringstream tinyPageObjsSS;
                tinyPageObjsSS << "page = " << iter->first << "'s objs: { count = " << iter->second.size() << "; ";
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    tinyPageObjsSS << *iter2 << ",";
                }

                std::string tinyPageObjsStr = tinyPageObjsSS.str();
                tinyTotalObjsSS << tinyPageObjsStr.substr(0, tinyPageObjsStr.length() - 1) << "}\n" << ALIGN6;
            }

            ss << tinyTotalObjsSS.str() << "\n";
            /****************small**********************/
            ss << "[Small]\n"
               << SEPERATOR << "\n"
               << ALIGN2 << "[[All slots pages]]:\n"
               << ALIGN4 << "slots total count: " << m_small_obj_pages.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream smallTotalPagesSS;
            for (auto iter = m_small_obj_pages.begin(); iter != m_small_obj_pages.end(); ++iter) {
                std::stringstream smallPageSS;
                smallPageSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second.size() << "; pages = ";
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    smallPageSS << *iter2 << ",";
                }

                std::string smallPageStr = smallPageSS.str();
                smallTotalPagesSS << smallPageStr.substr(0, smallPageStr.length() - 1) << "}\n" << ALIGN6;
            }

            ss << smallTotalPagesSS.str() << "\n";

            ss << ALIGN2 << "[[Free Objs]]:\n"
               << ALIGN4 << "slots total count: " << m_free_small_objs.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream smallTotalObjsSS;
            for (auto iter = m_free_small_objs.begin(); iter != m_free_small_objs.end(); ++iter) {
                std::stringstream smallSlotObjsSS;
                smallSlotObjsSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second.objs.size() << "\n" << ALIGN8;
                for (auto iter2 = iter->second.objs.begin(); iter2 != iter->second.objs.end(); ++iter2) {
                    std::stringstream smallPageObjsSS;
                    smallPageObjsSS << "page = " << iter2->first << "'s objs: { count = " << iter2->second.size() << "; ";
                    for (auto iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3) {
                        smallPageObjsSS << *iter3 << ",";
                    }
                    std::string smallPageObjsStr = smallPageObjsSS.str();
                    smallSlotObjsSS << smallPageObjsStr.substr(0, smallPageObjsStr.length() - 1) << "},\n" << ALIGN8;
                }

                std::string smallSlotObjsStr = smallSlotObjsSS.str();
                smallTotalObjsSS << smallSlotObjsStr.substr(0, smallSlotObjsStr.length() - 1) << "},\n" << ALIGN6;
            }

            ss << smallTotalObjsSS.str() << "\n";

            /****************big**********************/
            ss << "[Big]\n"
               << SEPERATOR << "\n"
               << ALIGN2 << "[[All slots pages]]:\n"
               << ALIGN4 << "slots total count: " << m_big_obj_pages.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream bigTotalPagesSS;
            for (auto iter = m_big_obj_pages.begin(); iter != m_big_obj_pages.end(); ++iter) {
                std::stringstream bigPageSS;
                bigPageSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second.size() << "; pages = ";
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    bigPageSS << *iter2 << ",";
                }

                std::string bigPageStr = bigPageSS.str();
                bigTotalPagesSS << bigPageStr.substr(0, bigPageStr.length() - 1) << "}\n" << ALIGN6;
            }

            ss << bigTotalPagesSS.str() << "\n";

            ss << ALIGN2 << "[[Free Objs]]:\n"
               << ALIGN4 << "slots total count: " << m_free_big_objs.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream bigTotalObjsSS;
            for (auto iter = m_free_big_objs.begin(); iter != m_free_big_objs.end(); ++iter) {
                std::stringstream bigSlotObjsSS;
                bigSlotObjsSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second.objs.size() << "\n" << ALIGN8;
                for (auto iter2 = iter->second.objs.begin(); iter2 != iter->second.objs.end(); ++iter2) {
                    std::stringstream bigPageObjsSS;
                    bigPageObjsSS << "page = " << iter2->first << "'s objs: { count = " << iter2->second.size() << "; ";
                    for (auto iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3) {
                        bigPageObjsSS << *iter3 << ",";
                    }
                    std::string bigPageObjsStr = bigPageObjsSS.str();
                    bigSlotObjsSS << bigPageObjsStr.substr(0, bigPageObjsStr.length() - 1) << "},\n" << ALIGN8;
                }

                std::string bigSlotObjsStr = bigSlotObjsSS.str();
                bigTotalObjsSS << bigSlotObjsStr.substr(0, bigSlotObjsStr.length() - 1) << "},\n" << ALIGN6;
            }

            ss << bigTotalObjsSS.str() << "\n";

            /****************bulk**********************/
            ss << "[Bulk]\n"
               << SEPERATOR << "\n"
               << ALIGN2 << "[[All slots pages]]:\n"
               << ALIGN4 << "slots total count: " << m_bulk_obj_pages.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream bulkTotalPagesSS;
            for (auto iter = m_bulk_obj_pages.begin(); iter != m_bulk_obj_pages.end(); ++iter) {
                std::stringstream bulkPageSS;
                bulkPageSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second.size() << "; pages = ";
                for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    bulkPageSS << *iter2 << ",";
                }

                std::string bulkPageStr = bulkPageSS.str();
                bulkTotalPagesSS << bulkPageStr.substr(0, bulkPageStr.length() - 1) << "}\n" << ALIGN6;
            }

            ss << bulkTotalPagesSS.str() << "\n";

            ss << ALIGN2 << "[[Free Objs]]:\n"
               << ALIGN4 << "slots total count: " << m_free_bulk_objs.size() << ";\n"
               << ALIGN4 << "slots:\n" << ALIGN6;
            std::stringstream bulkTotalObjsSS;
            for (auto iter = m_free_bulk_objs.begin(); iter != m_free_bulk_objs.end(); ++iter) {
                std::stringstream bulkSlotObjsSS;
                bulkSlotObjsSS << "slot = " << iter->first << "'s pages:{ count = " << iter->second->objs.size() << "\n" << ALIGN8;
                for (auto iter2 = iter->second->objs.begin(); iter2 != iter->second->objs.end(); ++iter2) {
                    std::stringstream bulkPageObjsSS;
                    bulkPageObjsSS << "page = " << iter2->first << "'s objs: { count = " << iter2->second.size() << "; ";
                    for (auto iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3) {
                        bulkPageObjsSS << *iter3 << ",";
                    }
                    std::string bulkPageObjsStr = bulkPageObjsSS.str();
                    bulkSlotObjsSS << bulkPageObjsStr.substr(0, bulkPageObjsStr.length() - 1) << "},\n" << ALIGN8;
                }

                std::string bulkSlotObjsStr = bulkSlotObjsSS.str();
                bulkTotalObjsSS << bulkSlotObjsStr.substr(0, bulkSlotObjsStr.length() - 1) << "},\n" << ALIGN6;
            }

            ss << bulkTotalObjsSS.str() << "\n";

            return ss.str();

#undef ALIGN2
#undef ALIGN4
#undef ALIGN6
#undef ALIGN8
        }
    }  // namespace common
}  // namespace netty
