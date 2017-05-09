/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_COMMON_UTILS_H
#define NET_COMMON_COMMON_UTILS_H

#include "common-def.h"

namespace common {
class CommonUtils {
public:
    static uctime_t get_current_time();
    static int set_nonblocking(int fd);
}; // class CommonUtils
}  // namespace common

#endif //NET_COMMON_COMMON_UTILS_H
