/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_THREAD_POOL_H
#define NET_COMMON_THREAD_POOL_H

#include <thread>
#include <vector>

namespace netty {
    namespace common {
        class ThreadPool {
        public:
            struct Task {
                std::function<void(void*)>  func;
                void                       *ctx;
            };

            ThreadPool(int threads_cnt);
            ~ThreadPool();

            void AddTask(Task t);

        private:
            std::vector<std::thread*> m_vThreadps;

        }; // class ThreadPool
    }  // namespace common
}  // namespace netty
#endif //NET_COMMON_THREAD_POOL_H
