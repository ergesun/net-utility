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
            std::vector<MemPool::MemObject*> memObjects;
            MemPool mp;
            /*******************tiny objs**********************/
            for (int i = 0; i < 10000; ++i) {
                auto needSize = rand() % 17;
                needSize = needSize ? needSize : 1;
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

            /*******************small objs**********************/
            memObjects.clear();
            for (int i = 0; i < 100000; ++i) {
                auto needSize = (32 + i) % 4097;
                needSize = needSize ? needSize : 32;
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

            /*******************big objs**********************/
            memObjects.clear();
            for (int i = 0; i < 1000; ++i) {
                auto needSize = (4097 + i) % (32 * 4096 + 1);
                needSize = needSize ? needSize : 4097;
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

            /*******************bulk objs**********************/
            memObjects.clear();
            for (int i = 0; i < 5; ++i) {
                auto needSize = 1024 * 1024 + i * 1024 * 1024;
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
        }
    }
}
