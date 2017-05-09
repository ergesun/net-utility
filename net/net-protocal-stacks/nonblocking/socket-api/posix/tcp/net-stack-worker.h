/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
#define NET_CORE_POSIX_TCP_NET_STACK_WORKER_H

#include "../../../../../../common/common-def.h"

namespace net {
/**
 * 本类负责具体的socket函数处理。
 */
class GCC_INTERNAL PosixTcpNetStackWorker {
public:
    /**
     * 错误: 返回false(无论是[socket错误或对端关闭]还是[codec校验错误])
     * 正常: 返回true(即便是遇到了EAGAIN，只要没有发生错误)
     * @return
     */
    bool Recv();
    /**
     * 发送缓冲队列里面的数据。
     * @return
     */
    bool Write();

private:

};
}

#endif //NET_CORE_POSIX_TCP_NET_STACK_WORKER_H
