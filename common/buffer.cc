/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "buffer.h"

namespace netty {
    namespace common {
        void Buffer::Refresh(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) {
            Pos = pos;
            Last = last;
            Start = start;
            End = end;
            MpObject = mpo;
        }
    }  // namespace common
}  // namespace netty
