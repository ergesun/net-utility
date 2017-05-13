/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>

#include "../common/thread-pool.h"

#include "thread-pool-test-case.h"

using namespace std;

namespace netty {
    namespace test {
        void ThreadPoolTest::Run() {
            static std::mutex s_mtx;
            common::ThreadPool tp;
            for (int i = 0; i < 100; ++i) {
                tp.AddTask([i](){
                    std::unique_lock<std::mutex> l(s_mtx);
                    cout << "index = " << i << endl;
                });
            }

            tp.WaitAll();
        }
    }
}
