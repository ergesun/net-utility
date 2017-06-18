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
         * [约定] 本buffer的使用规则需要统一如下：
         *   - last为空格，last最大到end为止
         *   - 总长度为  ：[start, end] --> 闭区间
         *   - 有效长度为：[pos, last]  --> 闭区间
         */
        class Buffer {
        public:
            Buffer() = default;
            Buffer(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) :
                Pos(pos), Last(last), Start(start), End(end), MpObject(mpo) {}
            ~Buffer() {
                Put();
            }

            Buffer(Buffer&&) = delete;
            Buffer(Buffer&) = delete;
            Buffer& operator=(Buffer&) = delete;

            inline void Put() {
                if (MpObject) {
                    MpObject->Put();
                } else {
                    DELETE_ARR_PTR(this->Start);
                }

                Refresh(nullptr, nullptr, nullptr, nullptr, nullptr);
            }

            inline void Refresh(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) {
                Pos = pos;
                Last = last;
                Start = start;
                End = end;
                MpObject = mpo;
            }

            inline bool Valid() {
                return Pos && Last && (uintptr_t)Last >= (uintptr_t)Pos;
            }

            inline void BZero() {
                Pos = nullptr;
                Last = nullptr;
                bzero(Start, (uintptr_t)End - (uintptr_t)Start + 1);
            }

            inline void RecvN(uint32_t n) {
                if (!Pos) {
                    Pos = Start;
                }

                if (Last) {
                    Last += n;
                } else {
                    Last = Start + n - 1;
                }
            }

            inline int32_t TotalLength() {
                return (int32_t)((uintptr_t)End - (uintptr_t)Start) + 1;
            }

            inline int32_t AvailableLength() {
                return (int32_t)((int64_t)(uintptr_t)Last - (int64_t)(uintptr_t)Pos) + 1;
            }

            inline size_t UnusedSize() {
                if (Last) {
                    return (size_t)(End - Last);
                } else {
                    return (size_t)(End - Start) + 1;
                }
            }

            uchar *Pos              = nullptr;
            uchar *Last             = nullptr;
            uchar *Start            = nullptr;
            uchar *End              = nullptr;
            MemPoolObject *MpObject = nullptr;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_BUFFER_H
