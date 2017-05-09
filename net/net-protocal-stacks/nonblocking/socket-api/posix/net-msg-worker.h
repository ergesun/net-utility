/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_ICONNECTION_H
#define NET_CORE_POSIX_TCP_ICONNECTION_H

namespace net {
/**
 * 本类负责数据的编解码/编解析，是对message的处理。具体的socket实现类要依赖于这个类来分发。
 */
class NetMessageWorker {
public:

    void HandleMessage();
};
}

#endif //NET_CORE_POSIX_TCP_ICONNECTION_H
