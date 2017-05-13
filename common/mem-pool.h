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

#include "timer.h"

#define BULK_PAGE_SIZE_THRESHOLD 134217728 // 128KiB

#define SMALL_OBJECT_RESIDENT_CNT    4096
#define BIG_OBJECT_RESIDENT_CNT      1024
#define EXTRA_OBJECT_CHECK_PERIOD    300 // 5 * 60seconds

namespace netty {
    namespace common {
        class MemPool {
        public:
            class MemObject {
            public:
                MemObject();
                ~MemObject();

                void* GetPointer() const;
                int32_t GetSize() const;

            private:
                uintptr_t m_slot_start_p;
                uintptr_t m_cur_obj_p;
                int32_t   m_slot_size;
                bool      m_bBulk;
            };

            typedef std::shared_ptr<MemObject> MemObjectRef;
            friend class MemObject;

        public:
            MemPool();
            /**
             *
             * @param sorc 每个槽位常驻的小对象的个数
             * @param borc 每个槽位常驻的大对象的个数
             * @param eocp 检查回收超额对象的定时器的周期，单位秒
             */
            MemPool(uint32_t sorc, uint32_t  borc, uint32_t eocp);
            /**
             *
             * @param sorc 每个槽位常驻的小对象的个数
             * @param borc 每个槽位常驻的大对象的个数
             * @param eocp 检查回收超额对象的定时器的周期，单位秒
             * @param bpt  巨大对象的界定阈值
             */
            MemPool(uint32_t sorc, uint32_t borc, uint32_t eocp, uint32_t bpt);
            ~MemPool();

            MemObjectRef Get(uint32_t size);
            void Put(MemObjectRef mor);

        private:
            void put(int32_t slot_size, uintptr_t slot_start_p, uintptr_t release_p);
            void check_recycle();
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

        private:
            /**
             * <槽大小，<4KiB页的首地址>>
             */
            std::unordered_map<int32_t, std::unordered_set<uintptr_t>> m_small_obj_pages; // 4KiB pages
            /**
             * <槽大小，<N个4KiB页大小的内存的首地址>>
             */
            std::unordered_map<int32_t, std::unordered_set<uintptr_t>> m_big_obj_pages; // n * 4KiB pages

            long m_sys_cacheline_size;
            long m_sys_page_size;
            uint32_t m_small_obj_resident_cnts;
            uint32_t m_big_obj_resident_cnts;
            uint32_t m_bult_page_threshold;
            uint32_t m_recycle_extra_page_period;
            Timer *m_recycle_check_timer;
            Timer::TimerCallback m_recycle_check_cb;
            Timer::Event m_recycle_check_ev;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_MEM_POOL_H
