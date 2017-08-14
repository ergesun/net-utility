/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <iostream>
#include "../common/buffer.h"
#include "../common/codec-utils.h"

#include "rcv-message.h"

namespace netty {
    namespace net {
        bool RcvMessage::DecodeMsgHeader(common::Buffer *buffer, Header *header) {
            if ((uint32_t)buffer->AvailableLength() < RcvMessage::HeaderSize()) {
                return false;
            }

            // 只简单校验MAGIC NO和payload size
            uint32_t offset = 0;
            header->magic = ByteOrderUtils::ReadUInt32(buffer->Pos + offset);
            if (header->magic != MESSAGE_MAGIC_NO) {
                std::cerr << "message magic no is invalid!" << std::endl;
                return false;
            }
            offset += sizeof(header->magic);
#if WITH_MSG_ID
#if BULK_MSG_ID
            header->id.ts = ByteOrderUtils::ReadUInt64(buffer->Pos + offset);
            offset += sizeof(uint64_t);
            header->id.seq = ByteOrderUtils::ReadUInt32(buffer->Pos + offset);
            offset += sizeof(header->id.seq);
#elif BIG_MSG_ID
            header->id = ByteOrderUtils::ReadUInt64(buffer->Pos + offset);
            offset += sizeof(header->id);
#else
            header->id = ByteOrderUtils::ReadUInt32(buffer->Pos + offset);
            offset += sizeof(header->id);
#endif
#endif
            header->len = ByteOrderUtils::ReadUInt32(buffer->Pos + offset);

            auto rc = header->len <= MAX_MSG_PAYLOAD_SIZE;
            if (!rc) {
                std::cerr << "message payload len is invalid!" << std::endl;
            }

            return rc;
        }
    } // namespace net
} // namespace netty

