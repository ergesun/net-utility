/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_IEVENTDRIVER_H
#define NET_CORE_IEVENTDRIVER_H

#include <vector>

#define EVENT_NONE       0
#define EVENT_READ       1
#define EVENT_WRITE      2

namespace net {
struct NetEvent {
    int fd;
    int mask;
};

/**
 * 事件驱动器。
 * EventDriver is a wrap of event mechanisms depends on different OS.
 * For example, Linux will use epoll(2), BSD will use kqueue(2) and select will
 * be used for worst condition.
 */
class IEventDriver {
public:
    virtual ~IEventDriver() {}
    virtual int init(int nevent) = 0;
    virtual int add_event(int fd, int cur_mask, int mask) = 0;
    virtual int del_event(int fd, int cur_mask, int del_mask) = 0;
    virtual int event_wait(std::vector<NetEvent> &events, struct timeval *tp) = 0;
}; // interface IEventDriver
}  // namespace net

#endif //NET_CORE_IEVENTDRIVER_H
