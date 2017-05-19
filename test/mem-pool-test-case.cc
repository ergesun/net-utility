/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cstring>
#include <iostream>
#include <vector>
#include "../common/mem-pool.h"

#include "mem-pool-test-case.h"

using namespace std;
using namespace netty::common;

namespace netty {
    namespace test {
        void MemPoolTest::Run() {
            MemPool mp;
            /*******************tiny objs**********************/
            std::vector<MemPool::MemObject*> memObjects;
            for (int i = 0; i < 10000; ++i) {
                auto needSize = 32 + i;
                auto memObject = mp.Get(needSize);
                memObjects.push_back(memObject);
                auto buf = memObject->Pointer<char>();
                auto size = memObject->Size();
                const char *str = "hello world!";
                memcpy(buf, str, strlen(str) + 1);
                cout << "buf = " << buf << ", needSize = " << needSize <<  ", get size = " << size << endl;
            }

            for (auto p : memObjects) {
                mp.Put(p);
            }
            cout << mp.DumpDebugInfo() << endl;

//            for (int i = 0; i < 100000; ++i) {
//                auto needSize = 32769 + i;
//                auto memObject = mp.Get(needSize);
//                auto buf = memObject->Pointer<char>();
//                auto size = memObject->Size();
//                const char *str = "hello world!";
//                memcpy(buf, str, strlen(str) + 1);
//                cout << "buf = " << buf << ", needSize = " << needSize <<  ", get size = " << size << endl;
//            }
        }
    }
}
