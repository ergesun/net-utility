/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_COMMON_BUFFER_H
#define NET_COMMON_BUFFER_H

#include "common-def.h"
#include "mem-pool.h"

namespace netty {
    namespace common {
        /**
         * 本buffer的使用规则需要统一如下：
         *   - last为空格，last最大到end为止
         *   - 总长度为  ：[start, end] --> 闭区间
         *   - 有效长度为：[pos, last)  --> 闭开区间
         */
        class Buffer {
        public:
            Buffer() = default;
            Buffer(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) :
                Pos(pos), Last(last), Start(start), End(end), MpObject(mpo) {}
            ~Buffer() {
                Pos   = nullptr;
                Last  = nullptr;
                Start = nullptr;
                End   = nullptr;
                MpObject->Put();
            }

            Buffer(Buffer&&) = delete;
            Buffer(Buffer&) = delete;
            Buffer& operator=(Buffer&) = delete;

            inline void Refresh(uchar *pos, uchar *last, uchar *start, v *e, MemPoolObject *mpo);

            uchar *Pos              = nullptr;
            uchar *Last             = nullptr;
            uchar *Start            = nullptr;
            uchar *End              = nullptr;
            MemPoolObject *MpObject = nullptr;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_BUFFER_H
