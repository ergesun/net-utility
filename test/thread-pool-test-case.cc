//
// Created by sunchao31 on 17-5-11.
//

#include <iostream>

#include "../common/thread-pool.h"

#include "thread-pool-test-case.h"

using namespace std;

namespace netty {
    namespace test {
        void ThreadPoolTest::Run() {
            common::ThreadPool tp;
            for (int i = 0; i < 100; ++i) {
                tp.AddTask([i](){
                    cout << "index = " << i << endl;
                });
            }
        }
    }
}
