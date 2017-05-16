/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_MEM_POOL_H
#define NET_COMMON_MEM_POOL_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <list>
#include <sstream>

#include "timer.h"

// TODO(sunchao): 添加一个和时间相关的统计机制，动态的对常驻对象数量进行调整。

/**
 * 大对象的界定阈值
 */
#define BIG_PAGE_SIZE_THRESHOLD               common::PAGE_SIZE
/**
 * 超大对象的界定阈值
 */
#define BULK_PAGE_SIZE_THRESHOLD              32768                      // 32 KiB
/**
 * 微小对象的界定阈值
 */
#define TINY_OBJECT_SIZE_THRESHOLD            16                         // bytes
/**
 * 微小对象保留的个数上限
 */
#define TINY_OBJECT_RESIDENT_CNT              512                        // 8 MiB
/**
 * 小对象槽的步长
 */
#define DEFAULT_SMALL_OBJECT_SLOT_FOOTSTEP    32                         // bytes (pow(2, 5) = 32 bytes)。值必须是2的N次幂。
/**
 * 每个小对象槽保留的个数上限
 */
#define ONE_SLOT_SMALL_OBJECT_RESIDENT_CNT    256
/**
 * 每个大对象槽保留的个数上限
 */
#define ONE_SLOT_BIG_OBJECT_RESIDENT_CNT      128
/**
 * 每个超大对象槽保留的个数上限
 */
#define ONE_SLOT_BULK_OBJECT_RESIDENT_CNT     common::CPUS_CNT
/**
 * 所有超大对象槽保留的个数和的上限
 */
#define ALL_SLOTS_BULK_OBJ_RESIDENT_CNT       common::CPUS_CNT
/**
 * 可留用保存的超大对象的大小上限
 */
#define RESIDENT_BULK_OBJ_MAX_SIZE            1048576                    // 1 MiB
/**
 * 各对象超出设定常驻个数的检查的周期
 */
#define EXTRA_OBJECT_CHECK_PERIOD             300                        // 5 * 60 seconds

#define DEFAULT_EXPAND_OBJ_CNT_FACTOR         32

namespace netty {
    namespace common {
        /**
         * 将内存按大小分成4种类别来管理。
         * 1. 微小型： <= 16 bytes
         * 2. 小型 ：  >= 16 bytes && <= page_size(一般4KiB)
         * 3. 大型 ：  >= page_size && <= BULK_PAGE_SIZE_THRESHOLD (默认配置为32KiB)
         * 4. 超大型： > BULK_PAGE_SIZE_THRESHOLD
         *
         * TODO(sunchao): 自己实现一个高效的hash-table，应用连接法，但是连接的内容是multi map，一旦hash code冲突，则组织到multi map中。
         *                (如果unordered_map已经是这个方案就不需要自己实现了。)
         */
        class MemPool {
        public:
            enum class MemObjectType {
                Tiny = 0,
                Small,
                Big,
                Bulk
            };

            class MemObject {
            public:
                MemObject(const MemObject&) = default;
                ~MemObject() = default;

                template <typename T>
                inline T* Pointer() const;
                inline uint32_t Size() const;
                inline MemObjectType Type() const;
                inline uint32_t SlotSize() const;
                inline uintptr_t ObjectPointerValue() const;
                inline uintptr_t SlotStartPointerValue() const;

            private:
                friend class MemPool;
                MemObject() = default;
                /**
                 *
                 * @param type
                 * @param slot_size
                 * @param obj_p
                 * @param slot_start_p
                 */
                MemObject(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t slotStartPv) :
                    m_type(type), m_slot_size(slotSize), m_obj_pv(objPv), m_slot_start_pv(slotStartPv) {}

                inline void refresh(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t slotStartPv);

            private:
                MemObjectType  m_type;
                uint32_t       m_slot_size; /* 空间大小(可能比申请的大) */
                uintptr_t      m_obj_pv; /* object pointer value */
                uintptr_t      m_slot_start_pv; /* slot start pointer value */
            };

            struct SlotObjPage {
                spin_lock_t                                         sl;
                // <页的首地址， <slot obj首地址>>
                std::unordered_map<uintptr_t, std::list<uintptr_t>> objs;
            };

            typedef std::shared_ptr<MemObject> MemObjectRef;
            friend class MemObject;

        public:
            /**
             * 使用默认构造函数，默认配置。
             */
            MemPool();
            /**
             *
             * @param torc
             * @param sorc 每个槽位常驻的小对象的个数
             * @param borc 每个槽位常驻的大对象的个数
             * @param eocp 检查回收超额对象的定时器的周期，单位秒
             */
            MemPool(uint32_t torc, uint32_t sorc, uint32_t  borc, uint32_t eocp);
            /**
             *
             * @param torc 每个槽位常驻的微小对象的个数
             * @param sorc 每个槽位常驻的小对象的个数
             * @param borc 每个槽位常驻的大对象的个数
             * @param eocp 检查回收超额对象的定时器的周期，单位秒
             * @param tpt  微小对象的界定阈值
             * @param bipt 大对象的界定阈值
             * @param bupt 巨大对象的界定阈值
             */
            MemPool(uint32_t torc,uint32_t sorc, uint32_t borc, uint32_t eocp, uint32_t tpt, uint32_t bipt, uint32_t bupt);
            ~MemPool();

            /**************通用简单接口***********************/
            /**
             * 获取一块指定大小的内存。
             * @param size
             * @return 失败的话MemObjectRef.get()为nullptr
             */
            MemObjectRef Get(uint32_t size);
            void Put(MemObjectRef mor);

            /**************复杂相对高性能接口***********************/
            // TODO(sunchao): 添加复杂高性能接口，最起码会减少很多if-else判断。 eg.
            // MemObjectRef GetTinyObject(uint32_t size);

        private:
            void put(MemObjectType type, int32_t slot_size, uintptr_t obj_pv, uintptr_t slot_start_pv);
            void check_objs();
            inline std::list<uintptr_t> split_mem_page(uint32_t slotSize, uintptr_t pagePv, uint32_t pageSize);
            inline MemObject* get_mem_object(MemObjectType type, uint32_t slotSize, uintptr_t objPv, uintptr_t slotStartPv);
            /**
             * 本函数的目的是找到一个整数，这个数要满足是2的exponent次幂的倍数，并且是比in大的所有数中最小的数。
             * @param in
             * @param exponent
             * @return
             */
            inline uint32_t RoundupToTheNextHighestMultipleOfExponent2(uint32_t in, uint32_t exponent) {
                uint32_t multiple_base = (uint32_t)2 << (exponent - 1);
                uint32_t mask = (0xFFFFFFFF >> exponent) << exponent;

                return (in & mask) + multiple_base;
            }

            inline uint32_t convert_slot_obj_size_to_slot_idx(uint32_t size) {
                return size / m_small_obj_slot_footstep_size;
            }

            inline uint32_t gen_expand_slot_page_size(uint32_t objSize, uint32_t expandObjCntFactor, uint32_t pageSize) {
                uint32_t expandCalSize = objSize * expandObjCntFactor;
                auto pageCnt = expandCalSize / pageSize;
                if (expandCalSize % pageSize) {
                    ++pageCnt;
                }

                return pageCnt * pageSize;
            }

            inline void alloc_page_objs(uint32_t size, uint32_t slotSize, std::unordered_set<uintptr_t> &pages,
                                        std::unordered_map<uintptr_t, std::list<uintptr_t>> &freeObjs);

            /**
             *
             * @param pages
             * @param slot_page_pv 出参
             * @param obj_pv 出参
             */
            inline void get_free_obj_from_slot_page(std::unordered_map<uintptr_t, std::list<uintptr_t>> &pages,
                                                    uintptr_t &slot_page_pv, uintptr_t &obj_pv);

            inline void get_suitable_bulk_page(uint32_t size, uintptr_t &pagePv, uint32_t &pageSize, float moreThanFactor = 2.0);

        private: // 有了槽位的思想，便对齐了。
            spin_lock_t m_tiny_obj_pages_lock = UNLOCKED;
            /**
             * <页的首地址>
             */
            std::unordered_set<uintptr_t> m_tiny_obj_pages; // 4KiB pages
            /**
             * <页的首地址， <tiny objs>>
             */
            std::unordered_map<uintptr_t, std::list<uintptr_t>> m_free_tiny_objs; // tiny objects

            /**
             * <槽大小(index == slotSize / footstep)，<页的首地址>>
             */
            std::unordered_map<uint32_t, std::unordered_set<uintptr_t>> m_small_obj_pages; // 4KiB pages
            /**
             * <槽大小(index == slotSize / footstep)， SmallPage>
             */
            std::unordered_map<uint32_t, SlotObjPage> m_free_small_objs; // small objects

            /**
             * <槽大小(n-->index)，<大对象内存的首地址>>
             */
            std::unordered_map<uint32_t, std::unordered_set<uintptr_t>> m_big_obj_pages; // [n] * 4KiB pages
            /**
             * <槽大小(n-->index)，<大对象内存的首地址>>
             */
            std::unordered_map<uint32_t , SlotObjPage> m_free_big_objs;

            spin_lock_t m_bulk_obj_pages_lock = UNLOCKED;
            /**
             * <槽大小(n-->index)，<超大页内存的首地址>>
             */
            std::unordered_map<uint32_t, std::unordered_set<uintptr_t>> m_bulk_obj_pages; // default more than 32KiB
            /**
             * <槽大小(n-->index)，<超大页内存的首地址>>
             */
            std::map<uint32_t, std::list<uintptr_t>> m_free_bulk_objs;

            // TODO(sunchao): 加个回收机制？
            std::list<MemObject*> m_free_mem_objs;

            uint32_t m_sys_cacheline_size;
            uint32_t m_sys_page_size;
            uint32_t m_one_slot_tiny_obj_resident_cnts;
            uint32_t m_one_slot_small_obj_resident_cnts;
            uint32_t m_small_obj_slot_footstep_size; // bytes
            uint32_t m_one_slot_big_obj_resident_cnts;
            uint32_t m_one_slot_bulk_obj_resident_cnts;
            uint32_t m_all_slots_bulk_obj_resident_cnts;
            uint32_t m_tiny_obj_threshold;
            uint32_t m_big_obj_threshold;
            uint32_t m_bulk_obj_threshold;
            uint32_t m_available_reserve_bulk_obj_max_size;
            uint32_t m_expand_obj_cnt_factor;
            uint32_t m_recycle_extra_page_period;
            Timer *m_recycle_check_timer;
            Timer::TimerCallback m_recycle_check_cb;
            Timer::Event m_recycle_check_ev;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_MEM_POOL_H
