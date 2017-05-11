/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_SPIN_LOCK_H
#define NET_COMMON_SPIN_LOCK_H

#define UNLOCKED                       0
#define LOCKED                         1

namespace netty {
    namespace common {
        /**
         * 使用时注意先初始化为UNLOCKED，或者你能保证编译器会初始化为0.
         */
        typedef unsigned long spin_lock_t;

        /**
         * 自旋锁。你应该应用在那些占用cpu时间短的逻辑上。
         * 注意：原子变量自带屏障，所以无需在使用的时候设置memory barrier，和std::unique_lock<std::mutex>一样使用即可。
         */
        class SpinLock {
        public:
            /**
             * 创建一个默认的自旋锁对象，构造函数自动Lock()加锁，析构函数自动解锁。
             * @param sl
             * @return
             */
            explicit SpinLock(spin_lock_t *const sl);

            /**
             * 创建一个默认的自旋锁对象，构造函数不会自动加锁。
             * @param sl
             * @param defer_lock
             * @return
             */
            SpinLock(spin_lock_t *const sl, bool defer_lock);

            /**
             * 构造一个指定争抢次数的自旋锁对象，构造函数自动Lock()加锁
             * 不懂不要用这个构造函数，直接用一个参数的构造函数。
             * @param sl
             * @param spin Lock()方法每一次抢锁到出让这段期间争抢的次数。
             * @return
             */
            SpinLock(spin_lock_t *const sl, uint8_t spin);

            /**
             * 构造一个指定争抢次数的自旋锁对象，构造函数不自动加锁。
             * 不懂不要用这个构造函数，直接用一个参数的构造函数。
             * @param sl
             * @param spin Lock()方法每一次抢锁到出让这段期间争抢的次数。
             * @return
             */
            SpinLock(spin_lock_t *const sl, uint8_t spin, bool defer_lock);

            ~SpinLock();

            bool TryLock();

            void Lock();

            void Unlock();

        private:
            SpinLock(const SpinLock &sl) = delete;

            const SpinLock &operator=(const SpinLock &sl) = delete;

        private:
            spin_lock_t *const m_psl;
            int m_iSpin = 1 << 11;
            bool m_bOwnLock = false;
        }; // class SpinLock
    }  // namespace common
}  // namespace netty
#endif //NET_COMMON_SPIN_LOCK_H
