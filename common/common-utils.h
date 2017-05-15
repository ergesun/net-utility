/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_COMMON_UTILS_H
#define NET_COMMON_COMMON_UTILS_H

#include "common-def.h"

namespace netty {
    namespace common {
        class CommonUtils {
        public:
            /**
             * 获取当前系统时间(unix epoch到现在的秒+纳秒数)。
             * @return
             */
            static uctime_t GetCurrentTime();

            /**
             * 设置fd为非阻塞。
             * @param fd
             * @return
             */
            static int SetNonBlocking(int fd);

            /**
             * posix_memalign的封装
             * @param align 对齐大小
             * @param size 申请的内存大小
             * @return
             */
            static void* PosixMemAlign(size_t align, size_t size);
        }; // class CommonUtils
    }  // namespace common
}  // namespace netty
#endif //NET_COMMON_COMMON_UTILS_H
