/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "common-utils.h"

#include "thread-pool.h"

namespace netty {
    namespace common {
        ThreadPool::ThreadPool(int threads_cnt) {
            m_iActiveWorkersCnt.store(0);
            threads_cnt = threads_cnt > 0 ? threads_cnt : common::CPUS_CNT * 2;
            m_vThreadps.reserve(threads_cnt);
            for (int i = 0; i < threads_cnt; ++i) {
                m_vThreadps.push_back(new std::thread(std::bind(&ThreadPool::proc, this)));
            }
        }

        ThreadPool::~ThreadPool() {
            m_bStopping = true;
            // 添加空的task以防止bq中没有内容无法停止的情况发生。
            Task empty_task;
            auto size = m_vThreadps.size();
            for (int i = 0; i < size; ++i) {
                AddTask(empty_task);
            }

            for (auto pt : m_vThreadps) {
                pt->join();
                delete pt;
            }
        }

        void ThreadPool::WaitAll() {
            std::unique_lock<std::mutex> l(m_mtxActiveWorkerCnt);
            while (!m_bgqTasks.Empty() || 0 != m_iActiveWorkersCnt.load()) {
                m_cvActiveWorkerCnt.wait(l);
            }
        }

        void ThreadPool::WaitAllUntilTimeAt(uctime_s duration_since_epoch) {
            std::unique_lock<std::mutex> l(m_mtxActiveWorkerCnt);
            using namespace std::chrono;
            time_point<system_clock, nanoseconds> tp(nanoseconds(duration_since_epoch.get_total_nsecs()));
            bool first = true;
            while (first && (!m_bgqTasks.Empty() && 0 != m_iActiveWorkersCnt.load())) {
                m_cvActiveWorkerCnt.wait_until(l, tp);
                first = false;
            }
        }

        void ThreadPool::WaitAllUntilAfter(uctime_s duration) {
            auto now = common::CommonUtils::get_current_time();
            now += duration;

            WaitAllUntilTimeAt(now);
        }

        void ThreadPool::proc() {
            while (true) {
                auto task = m_bgqTasks.Pop();
                // 此处无需担心m_stopping所判断的成员因为优化导致不能读取到内存数据的情况，
                // 因为BlockingGetQueue::Pop内部有内存屏障作用。
                if (m_bStopping) {
                    break;
                }

                if (task) {
                    m_iActiveWorkersCnt++;
                    task();
                    m_iActiveWorkersCnt--;
                }

                m_cvActiveWorkerCnt.notify_one();
            }
        }
    }
}

