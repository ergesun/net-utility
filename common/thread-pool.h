/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_THREAD_POOL_H
#define NET_COMMON_THREAD_POOL_H

#include <thread>
#include <vector>
#include <unordered_set>

#include "blocking-get-queue.h"
#include "common-def.h"
#include "spin-lock.h"

namespace netty {
    namespace common {
        /**
         * 当前仅支持FIFO调度的线程池。
         * TODO(sunchao): 1、添加调度策略 2、考虑task相关联性，关联task分配到不同的线程 3、cpu亲缘性绑定？再思考，感觉没必要。
         */
        class ThreadPool {
        public:
            typedef std::function<void(void)> Task, *HeldTask, *TaskId;

            /**
             *
             * @param threads_cnt 线程个数。如果小于等于0则为cpu核数 * 2个。
             */
            ThreadPool(int threads_cnt = 0) {
                threads_cnt = threads_cnt > 0 ? threads_cnt : common::CPUS_CNT * 2;
                m_vThreadps.resize(threads_cnt);
                for (int i = 0; i < threads_cnt; ++i) {
                    m_vThreadps.push_back(new std::thread(std::bind(&ThreadPool::proc, this)));
                }
            }

            ~ThreadPool() {
                m_stopping = true;
                // 添加一个空的task以防止bq中没有内容无法停止的情况发生。
                Task empty_task;
                AddTask(empty_task);
                for (auto pt : m_vThreadps) {
                    pt->join();
                    delete pt;
                }
            }

            /**
             * 添加一个任务到线程池中执行。
             * 注意：此task是可以指定等待的(TODO(sunchao):加上取消接口)，此task用户不可以释放，要持有直到结束。
             * @param t 添加的task。
             * @return task的id用来等待或cancel。
             */
            inline TaskId AddHeldTask(HeldTask t) {
                m_bgq_can_cancel_tasks.Push(std::make_pair(t, t));
                return t;
            }

            inline void AddTask(Task t) {
                m_bgq_tasks.Push(t);
            }

            /**
             * 等待指定的task完成。
             * @param tid
             */
            inline void WaitTask(TaskId tid) {

            }

            /**
             * 等待所有的task完成。
             */
            inline void WaitAll() {

            }

        private:
            void proc() {
                while (true) {
                    auto task = m_bgq_can_cancel_tasks.Pop();
                    // 此处无需担心m_stopping所判断的成员因为优化导致不能读取到内存数据的情况，因为有BlockingQueue::Pop屏障。
                    if (m_stopping) {
                        break;
                    }

                    if (task) {
                        task();
                    }
                }
            }

        private:
            typedef std::pair<HeldTask, TaskId> TaskCtx;

            bool m_stopping = false;
            std::vector<std::thread*> m_vThreadps;
            common::BlockingGetQueue<TaskCtx> m_bgq_can_cancel_tasks;
            std::unordered_set<TaskId> m_hsTaskIds;

            common::BlockingGetQueue<Task> m_bgq_tasks;
            bool m_is_cancel_tasks_turn = false;
            spin_lock_t m_sl_turn_lock = UNLOCKED;
        }; // class ThreadPool
    }  // namespace common
}  // namespace netty
#endif //NET_COMMON_THREAD_POOL_H
