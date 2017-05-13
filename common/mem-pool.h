/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_MEM_POOL_H
#define NET_COMMON_MEM_POOL_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace netty {
    namespace common {
        class MemPool {
        public:
            class MemObject {
            public:
                MemObject();
                ~MemObject();

            private:

            };

            typedef std::shared_ptr<MemObject> MemObjectRef;

        public:
            MemPool();
            ~MemPool();

            MemObjectRef Get(uint32_t size);

        private:
            void Put(int32_t slot_size, char slot_start_p, char *release_p);

        private:
            std::unordered_map<int32_t, std::unordered_set<char*>> m_size_pages;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_MEM_POOL_H
