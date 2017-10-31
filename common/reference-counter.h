/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NETTY_COMMON_REFERENCE_COUNTER_H
#define NETTY_COMMON_REFERENCE_COUNTER_H

#include <stdint-gcc.h>

namespace netty {
    namespace common {
        /**
         * Thread-safe.
         * 维护一个引用计数的类。
         */
        class ReferenceCounter {
        public:
            explicit ReferenceCounter(uint32_t ref) : m_iRef(ref) {}
            virtual ~ReferenceCounter() = default;

            /**
             * 添加一个引用计数。
             */
            virtual void AddRef();

            /**
             * 释放一个引用计数。
             */
            virtual void Release();

        protected:
            /**
             * 指定一个引用计数。
             * @param val
             */
            void SetRef(int32_t val);

            /**
             * 获取当前引用计数。
             * @return 引用计数。
             */
            int32_t GetRef();

        private:
            int32_t        m_iRef = 0;
        }; // class ReferenceCounter
    } // namespace common
} // namespace netty

#endif //NETTY_COMMON_REFERENCE_COUNTER_H
