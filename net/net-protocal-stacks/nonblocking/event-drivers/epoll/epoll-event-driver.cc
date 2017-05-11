/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cerrno>
#include <sys/epoll.h>
#include <cstring>
#include <iostream>
#include <cassert>

#include "epoll-event-driver.h"

namespace netty {
    namespace net {
        int EpollEventDriver::init(int max_events) {
            assert(max_events > 0);
            m_max_events = max_events;
            m_events = (struct epoll_event *) (malloc(max_events * sizeof(struct epoll_event)));
            if ((m_epfd = epoll_create(max_events)) < 0) {
                auto err = errno;
                std::cerr << "epoll_create err with err = " << strerror(err) << std::endl;
                return -1;
            }
            return 0;
        }

        int EpollEventDriver::add_event(int fd, int cur_mask, int mask) {
            int op = (cur_mask == EVENT_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

            // 放置到epoll中
            struct epoll_event ee;
            // 设置epoll事件。
            ee.data.fd = fd;
            ee.events = EPOLLHUP | EPOLLERR | EPOLLET;

            mask |= cur_mask;
            if (mask & EVENT_READ) {
                ee.events |= EPOLLIN;
            }
            if (mask & EVENT_WRITE) {
                ee.events |= EPOLLOUT;
            }

            if (-1 == epoll_ctl(m_epfd, op, fd, &ee)) {
                auto err = errno;
                std::cerr << "epoll_ctl err with err = " << strerror(err) << std::endl;
                return -1;
            }

            return 0;
        }

        int EpollEventDriver::del_event(int fd, int cur_mask, int del_mask) {
            struct epoll_event ee;

            ee.data.fd = fd;
            ee.events = EPOLLHUP | EPOLLERR | EPOLLET;

            int mask = cur_mask & (~del_mask);
            if (mask & EVENT_READ) {
                ee.events |= EPOLLIN;
            }
            if (mask & EVENT_WRITE) {
                ee.events |= EPOLLOUT;
            }

            if (mask != EVENT_NONE) {
                if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ee) < 0) {
                    auto err = errno;
                    std::cerr << __func__ << " epoll_ctl: modify fd=" << fd << " mask=" << mask
                              << " failed." << strerror(err) << std::endl;
                    return -1;
                }
            } else {
                /* Note, Kernel < 2.6.9 requires a non null event pointer even for
                 * EPOLL_CTL_DEL. */
                if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &ee) < 0) {
                    auto err = errno;
                    std::cerr << __func__ << " epoll_ctl: delete fd=" << fd
                              << " failed." << strerror(err) << std::endl;
                    return -1;
                }
            }

            return 0;
        }

        int EpollEventDriver::event_wait(std::vector<NetEvent> &events, struct timeval *tp) {
            assert(events.size() >= m_max_events);
            int nevents = epoll_wait(m_epfd, m_events, m_max_events,
                                     tp ? (int) ((tp->tv_sec * 1000 + tp->tv_usec / 1000)) : -1);
            if (nevents > 0) {
                int revents = 0;
                events.resize((unsigned long) nevents);
                for (int i = 0; i < nevents; i++) {
                    int mask = 0;
                    revents = m_events[i].events;
                    if (revents & EPOLLIN) {
                        mask |= EVENT_READ;
                    }
                    if (revents & EPOLLOUT) {
                        mask |= EVENT_WRITE;
                    }
                    if (revents & EPOLLERR || revents & EPOLLHUP) {
                        mask |= EVENT_READ | EVENT_WRITE;
                    }

                    events[i].fd = m_events[i].data.fd;
                    events[i].mask = mask;
                }
            }

            return nevents;
        }
    } // namespace net
} // namespace netty
