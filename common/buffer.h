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
         * Not thread-safe.
         * [约定] 本buffer的使用规则需要统一如下：
         *   - last为空格，last最大到end为止
         *   - 总长度为  ：[start, end] --> 闭区间
         *   - 有效长度为：[pos, last]  --> 闭区间
         */
        class Buffer {
        public:
            Buffer() = default;
            Buffer(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) :
                m_pPos(pos), m_pLast(last), m_pStart(start), m_pEnd(end), m_pMpObject(mpo) {
                check_available();
            }
            ~Buffer() {
                Put();
            }

            Buffer(Buffer&&) = delete;
            Buffer(Buffer&) = delete;
            Buffer& operator=(Buffer&) = delete;

            inline void Put() {
                Refresh(nullptr, nullptr, nullptr, nullptr, nullptr);
            }

            inline void Refresh(uchar *pos, uchar *last, uchar *start, uchar *end, MemPoolObject *mpo) {
                if (m_pMpObject) {
                    m_pMpObject->Put();
                } else {
                    DELETE_ARR_PTR(m_pStart);
                }

                m_pMpObject = mpo;
                m_pPos = pos;
                m_pLast = last;
                m_pStart = start;
                m_pEnd = end;
                check_available();
            }

            inline bool Valid() {
                return m_bAvailable;
            }

            inline void BZero() {
                m_pPos = nullptr;
                m_pLast = nullptr;
                bzero(m_pStart, m_pEnd - m_pStart + 1);
                m_bAvailable = false;
            }

            /**
             * 向后移动有效头指针Pos
             * @param n
             */
            inline void MoveHeadBack(uint32_t n) {
                if (UNLIKELY(!m_pPos || !m_pLast)) {
                    m_bAvailable = false;
                    return;
                }
                m_bAvailable = ((m_pLast - m_pPos) > (n - 1));
                m_pPos += n;
            }

            /**
             * 向后移动有效尾指针Last
             * @param n
             */
            inline void MoveTailBack(uint32_t n) {
                if (m_pLast) {
                    m_pLast += n;
                } else {
                    m_pLast = m_pStart + n - 1;
                    check_available();
                }
            }

            inline int32_t TotalLength() const {
                if (UNLIKELY(!m_pStart || !m_pEnd)) {
                    return 0;
                }

                return (int32_t)(m_pEnd - m_pStart) + 1;
            }

            inline int32_t AvailableLength() const {
                if (!m_bAvailable) {
                    return 0;
                }
                if (UNLIKELY(!m_pLast || !m_pPos)) {
                    return 0;
                }

                return (int32_t)(m_pLast - m_pPos) + 1;
            }

            inline size_t UnusedSize() const {
                if (!m_bAvailable) {
                    return 0;
                }

                if (LIKELY(m_pLast)) {
                    return (size_t)(m_pEnd - m_pLast);
                } else {
                    return (size_t)(m_pEnd - m_pStart) + 1;
                }
            }

            inline void Reset() {
                Refresh(nullptr, nullptr, nullptr, nullptr, nullptr);
            }

            inline void SetPos(uchar *p) {
                m_pPos = p;
                check_available();
            }

            inline void SetLast(uchar *l) {
                m_pLast = l;
                check_available();
            }

            inline void SetStart(uchar *s) {
                m_pStart = s;
            }

            inline void SetEnd(uchar *e) {
                m_pEnd = e;
            }

            inline uchar* GetPos() const {
                return m_pPos;
            }

            inline uchar* GetLast() const {
                return m_pLast;
            }

            inline uchar* GetStart() const {
                return m_pStart;
            }

            inline uchar* GetEnd() const {
                return m_pEnd;
            }

        private:
            inline void check_available() {
                if (!m_pStart || !m_pEnd || !m_pPos || !m_pLast) {
                    m_bAvailable = false;
                    return;
                }

                m_bAvailable = (uintptr_t)m_pPos <= (uintptr_t)m_pLast;
            }

        private:
            uchar             *m_pPos              = nullptr;
            uchar             *m_pLast             = nullptr;
            uchar             *m_pStart            = nullptr;
            uchar             *m_pEnd              = nullptr;
            MemPoolObject     *m_pMpObject         = nullptr;
            bool               m_bAvailable        = false;
        };
    }  // namespace common
}  // namespace netty

#endif //NET_COMMON_BUFFER_H
