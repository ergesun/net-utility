/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <time.h>
#include <fcntl.h>
#include <iostream>

#include "common-utils.h"

namespace netty {
    namespace common {
        uctime_t CommonUtils::GetCurrentTime() {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);

            return uctime_t(ts);
        }

        int CommonUtils::SetNonBlocking(int fd) {
            int opts;
            int err;
            if ((opts = fcntl(fd, F_GETFL)) == -1) {
                err = errno;
                std::cerr << "get fd opts err = " << err << std::endl;

                return -1;
            }

            opts = opts | O_NONBLOCK;
            if ((err = fcntl(fd, F_SETFL, opts)) == -1) {
                err = errno;
                std::cerr << "set fd O_NONBLOCK err = " << err << std::endl;
                return -1;
            }

            return 0;
        }

        void* CommonUtils::PosixMemAlign(size_t align, size_t size) {
            void *pln;
            if (int ret = posix_memalign(&pln, align, size)) {
                return nullptr;
            }

            return pln;
        }
    } // namespace common
} // namespace netty
