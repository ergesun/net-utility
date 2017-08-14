/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_THREAD_POOL_H
#define NET_COMMON_THREAD_POOL_H

#include <thread>
#include <vector>
#include <unordered_set>
#include <atomic>

#include "blocking-queue.h"
#include "common-def.h"
#include "spin-lock.h"

namespace netty {
    namespace common {
        /**
         * 当前仅支持FIFO调度的线程池。
         * TODO(sunchao): 1、添加调度策略 2、考虑task相关联性，关联task分配到不同的线程 3、cpu亲缘性绑定？再思考，感觉没必要。
         *                4、添加指定的task的wait task、cancel task
         */
        class ThreadPool {
        public:
            struct Task {
                Task() = default;
                /**
                 *
                 * @param callback 回调函数
                 */
                Task(std::function<void(void*)> callback) {
                    action = callback;
                }

                /**
                 *
                 * @param callback 回调函数
                 * @param ctx 回调传回的上下文
                 */
                Task(std::function<void(void*)> callback, void *ctx) {
                    action = callback;
                    this->ctx = ctx;
                }

                std::function<void(void*)>  action;
                void                       *ctx = nullptr;
            };

            /**
             *
             * @param threads_cnt 线程个数。如果小于等于0则为cpu核数 * 2个。
             */
            ThreadPool(int threads_cnt = 0);

            /**
             * 会join所有threads。
             */
            ~ThreadPool();

            /**
             * 添加一个任务到线程池中执行。
             * @param t 添加的task。
             */
            inline void AddTask(Task t) {
                m_bqTasks.Push(t);
            }

            /**
             * 等待所有的task完成。
             */
            void WaitAll();

            /**
             * 等待所有都完成或者指定的时间到了。
             * @param duration_since_epoch epoch开始到现在的duration。
             */
            void WaitAllUntilTimeAt(uctime_s duration_since_epoch);

            /**
             * 等待所有都完成或者指定的时间到了。
             * @param duration 从现在开始最多等待的持续时间。
             */
            void WaitAllUntilAfter(uctime_s duration);

        private:
            void proc();

        private:
            bool                           m_bStopping = false;
            std::vector<std::thread*>      m_vThreadps;
            common::BlockingQueue<Task>    m_bqTasks;
            std::atomic<int>               m_iActiveWorkersCnt;
            std::mutex                     m_mtxActiveWorkerCnt;
            std::condition_variable        m_cvActiveWorkerCnt;
        }; // class ThreadPool
    }  // namespace common
}  // namespace netty
#endif //NET_COMMON_THREAD_POOL_H
