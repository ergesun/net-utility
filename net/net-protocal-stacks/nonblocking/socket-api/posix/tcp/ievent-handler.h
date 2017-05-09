/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_POSIX_TCP_IEVENTHANDLER_H
#define NET_CORE_POSIX_TCP_IEVENTHANDLER_H

namespace net {
class IEventHandler {
public:
    virtual ~IEventHandler() {}
    virtual int HandleReadEvent() = 0;
    virtual int HandleWriteEvent() = 0;
}; // interface IEventHandler
}  // namespace net

#endif //NET_CORE_POSIX_TCP_IEVENTHANDLER_H
