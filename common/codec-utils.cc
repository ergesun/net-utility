/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include "codec-utils.h"

namespace netty {
    namespace common {
        void LittleEndianCodecUtils::WriteUInt16(uchar *buf, uint16_t n) {
            buf[1] = (uchar)n;
            n >>= 8;
            buf[0] = (uchar)n;
        }

        void LittleEndianCodecUtils::WriteUInt32(uchar *buf, uint32_t n) {
            buf[3] = (uchar)n;
            n >>= 8;
            buf[2] = (uchar)n;
            n >>= 8;
            buf[1] = (uchar)n;
            n >>= 8;
            buf[0] = (uchar)n;
        }

        void LittleEndianCodecUtils::WriteUInt64(uchar *buf, uint64_t n) {
            buf[7] = (uchar)n;
            n >>= 8;
            buf[6] = (uchar)n;
            n >>= 8;
            buf[5] = (uchar)n;
            n >>= 8;
            buf[4] = (uchar)n;
            n >>= 8;
            buf[3] = (uchar)n;
            n >>= 8;
            buf[2] = (uchar)n;
            n >>= 8;
            buf[1] = (uchar)n;
            n >>= 8;
            buf[0] = (uchar)n;
        }

        uint16_t LittleEndianCodecUtils::ReadUInt16(uchar *buf) {
            uint16_t n = buf[0];

            n <<= 8;
            n |= buf[1];

            return n;
        }

        uint32_t LittleEndianCodecUtils::ReadUInt32(uchar *buf) {
            uint32_t n = buf[0];

            n <<= 8;
            n |= buf[1];
            n <<= 8;
            n |= buf[2];
            n <<= 8;
            n |= buf[3];

            return n;
        }

        uint64_t LittleEndianCodecUtils::ReadUInt64(uchar *buf) {
            uint64_t n = buf[0];

            n <<= 8;
            n |= buf[1];
            n <<= 8;
            n |= buf[2];
            n <<= 8;
            n |= buf[3];
            n <<= 8;
            n |= buf[4];
            n <<= 8;
            n |= buf[5];
            n <<= 8;
            n |= buf[6];
            n <<= 8;
            n |= buf[7];

            return n;
        }
    }
}
