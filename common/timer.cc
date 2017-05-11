/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#include <cassert>

#include "common-utils.h"
#include "timer.h"

namespace netty {
    namespace common {
        Timer::~Timer() {
            m_pWorkThread->join();
            delete m_pWorkThread;
        }

        void Timer::Start() {
            SpinLock l(&m_thread_safe_sl);
            if (!m_stop) {
                return;
            }
            m_stop = false;

            if (!m_pWorkThread) {
                m_pWorkThread = new std::thread(std::bind(&Timer::process, this));
            }
        }

        void Timer::Stop() {
            SpinLock l(&m_thread_safe_sl);
            m_stop = true;
            UnsubscribeAllEvent();
            m_cv.notify_one();
        }

        Timer::EventId Timer::SubscribeEventAt(uctime_t when, Event &ev) {
            assert(ev.callback);
            SpinLock l(&m_thread_safe_sl);
            TimerEvents::value_type te_pair(when, ev);
            auto insert_pos = m_mapSubscribedEvents.insert(te_pair);
            EventsTable::value_type et_pair(ev.callback, insert_pos);
            m_mapEventsEntry.insert(et_pair);
            if (insert_pos == m_mapSubscribedEvents.begin()) {
                m_cv.notify_one();
            }

            return ev.callback;
        }

        Timer::EventId Timer::SubscribeEventAfter(uctime_t duration, Event &ev) {
            assert(ev.callback);
            auto now = CommonUtils::get_current_time();
            now += duration;
            return SubscribeEventAt(now, ev);
        }

        bool Timer::UnsubscribeEvent(EventId eventId) {
            assert(eventId);
            SpinLock l(&m_thread_safe_sl);
            auto ev = m_mapEventsEntry.find(eventId);
            if (m_mapEventsEntry.end() != ev) {
                m_mapSubscribedEvents.erase(ev->second);
                m_mapEventsEntry.erase(ev);
            }
        }

        void Timer::UnsubscribeAllEvent() {
            SpinLock l(&m_thread_safe_sl);
            for (auto ev_entry : m_mapEventsEntry) {
                m_mapSubscribedEvents.erase(ev_entry.second);
                m_mapEventsEntry.erase(ev_entry.first);
            }
        }

        void Timer::process() {
            std::unique_lock<std::mutex> ml(m_evs_mtx);
            while (!m_stop) { // 锁有屏障作用，无需担心m_stop多线程访问的问题。
                SpinLock sl(&m_thread_safe_sl);
                while (!m_mapSubscribedEvents.empty()) {
                    auto min = m_mapSubscribedEvents.begin();
                    if (min->first > CommonUtils::get_current_time()) {
                        break;
                    }

                    (*(min->second.callback))(min->second.ctx);
                    m_mapEventsEntry.erase(min->second.callback);
                    m_mapSubscribedEvents.erase(min);
                }

                if (m_mapSubscribedEvents.empty()) {
                    sl.Unlock();
                    m_cv.wait(ml);
                } else {
                    auto min = m_mapSubscribedEvents.begin();
                    sl.Unlock();
                    using namespace std::chrono;
                    time_point<system_clock, nanoseconds> tp(nanoseconds(min->first.get_total_nsecs()));
                    m_cv.wait_until(ml, tp);
                }
            }
        }
    }  // namespace common
}  // namespace netty
