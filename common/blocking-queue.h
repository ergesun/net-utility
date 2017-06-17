/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_BLOCKING_GET_QUEUE_H
#define NET_COMMON_BLOCKING_GET_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

namespace netty {
    namespace common {
        /**
         * 阻塞队列。
         */
        template <typename T>
        class BlockingQueue {
        public:
            /**
             *
             * @param queueMaxSize 保有的队列元素的最大个数，0为无限制。
             */
            BlockingQueue(uint32_t queueMaxSize = 0) : m_iQueueMaxSize(queueMaxSize) {}

            bool TryPop(T& item) {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_queue.empty()) {
                    return false;
                }

                item = std::move(m_queue.front());
                m_queue.pop();
                if (0 != m_iQueueMaxSize) {
                    l.unlock();
                    m_cond.notify_one();
                }

                return true;
            }

            T Pop() {
                std::unique_lock<std::mutex> l(m_mutex);
                while (m_queue.empty()) {
                    m_cond.wait(l);
                }
                auto item = std::move(m_queue.front());
                m_queue.pop();
                if (0 != m_iQueueMaxSize) {
                    l.unlock();
                    m_cond.notify_one();
                }
                return item;
            }

            void Pop(T& item) {
                std::unique_lock<std::mutex> l(m_mutex);
                while (m_queue.empty()) {
                    m_cond.wait(l);
                }
                item = std::move(m_queue.front());
                m_queue.pop();
                if (0 != m_iQueueMaxSize) {
                    l.unlock();
                    m_cond.notify_one();
                }
            }

            /**
             * 不需要你保证对象不被释放，但对象内部的元素不可以被释放，否则元素内容将无效。
             * @param item
             * @return 成功返回true
             */
            bool TryPush(T &item) {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_iQueueMaxSize != 0 && m_queue.size() >= m_iQueueMaxSize) {
                    return false;
                }

                m_queue.push(item);
                l.unlock();
                m_cond.notify_one();
                return true;
            }

            /**
             * 不需要你保证对象不被释放，但对象内部的元素不可以被释放，否则元素内容将无效。
             * @param item
             * @return 成功返回true
             */
            bool TryPush(T&& item) {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_iQueueMaxSize != 0 && m_queue.size() >= m_iQueueMaxSize) {
                    return false;
                }

                m_queue.push(std::move(item));
                l.unlock();
                m_cond.notify_one();

                return true;
            }

            /**
             * 不需要你保证对象不被释放，但对象内部的元素不可以被释放，否则元素内容将无效。
             * @param item
             */
            void Push(T &item) {
                std::unique_lock<std::mutex> l(m_mutex);
                while (m_iQueueMaxSize != 0 && m_queue.size() >= m_iQueueMaxSize) {
                    m_cond.wait(l);
                }

                m_queue.push(item);
                l.unlock();
                m_cond.notify_one();
            }

            /**
             * 不需要你保证对象不被释放，但对象内部的元素不可以被释放，否则元素内容将无效。
             * @param item
             */
            void Push(T&& item) {
                std::unique_lock<std::mutex> l(m_mutex);
                while (m_iQueueMaxSize != 0 && m_queue.size() >= m_iQueueMaxSize) {
                    m_cond.wait(l);
                }

                m_queue.push(std::move(item));
                l.unlock();
                m_cond.notify_one();
            }

            bool Empty() {
                std::unique_lock<std::mutex> l(m_mutex);
                return m_queue.empty();
            }

            uint64_t Size() {
                std::unique_lock<std::mutex> l(m_mutex);
                return m_queue.size();
            }

            void Clear() {
                std::unique_lock<std::mutex> l(m_mutex);
                while (!m_queue.empty()) {
                    m_queue.pop();
                }
            }

        private:
            uint32_t m_iQueueMaxSize = 0;
            std::queue<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_cond;
        }; /* class BlockingQueue */
    } // namespace common
} // namespace netty

#endif //NET_COMMON_BLOCKING_GET_QUEUE_H
