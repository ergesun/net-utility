/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cstring>
#include <iostream>
#include "../common/mem-pool.h"

#include "mem-pool-test-case.h"

using namespace std;
using namespace netty::common;

namespace netty {
    namespace test {
        void MemPoolTest::Run() {
            MemPool mp;
            for (int i = 0; i < 10000; ++i) {
                auto memObject = mp.Get(22 + i);
                auto buf = memObject->Pointer<char>();
                auto size = memObject->Size();
                const char *str = "hello world!";
                memcpy(buf, str, strlen(str) + 1);
                cout << "buf = " << buf << ", size = " << size << endl;
            }
        }
    }
}
