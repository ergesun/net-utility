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
            if (!m_stop) {
                Stop();
            }

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
            UnsubscribeAllEvent();
            SpinLock l(&m_thread_safe_sl);
            m_stop = true;
            m_cv.notify_one();
        }

        Timer::EventId Timer::SubscribeEventAt(uctime_t when, Event &ev) {
            assert(ev.callback);
            auto evId = EventId(when, ev.callback);
            SpinLock l(&m_thread_safe_sl);
            // 如果已存在，直接返回。同一时间点的同一回调只能订阅一次。
            auto findPos = m_mapSubscribedEvents.find(when);
            for (; findPos != m_mapSubscribedEvents.end(); ++findPos) {
                if (findPos->second.callback == ev.callback) {
                    evId.when.nsec = 0;
                    evId.when.sec = 0;
                    return evId;
                }
            }

            TimerEvents::value_type te_pair(when, ev);
            auto insert_pos = m_mapSubscribedEvents.insert(te_pair);
            EventsTable::value_type et_pair(evId, insert_pos);
            m_mapEventsEntry.insert(et_pair);
            if (insert_pos == m_mapSubscribedEvents.begin()) {
                m_cv.notify_one();
            }

            return EventId(when, ev.callback);
        }

        Timer::EventId Timer::SubscribeEventAfter(uctime_t duration, Event &ev) {
            assert(ev.callback);
            auto now = CommonUtils::GetCurrentTime();
            now += duration;

            return SubscribeEventAt(now, ev);
        }

        bool Timer::UnsubscribeEvent(EventId eventId) {
            if (0 == eventId.when || nullptr == eventId.how) {
                return false;
            }

            SpinLock l(&m_thread_safe_sl);
            auto ev = m_mapEventsEntry.find(eventId);
            if (m_mapEventsEntry.end() != ev) {
                m_mapSubscribedEvents.erase(ev->second);
                m_mapEventsEntry.erase(ev);
                return true;
            }

            return false;
        }

        void Timer::UnsubscribeAllEvent() {
            SpinLock l(&m_thread_safe_sl);
            m_mapEventsEntry.clear();
            m_mapSubscribedEvents.clear();
        }

        void Timer::process() {
            std::unique_lock<std::mutex> ml(m_evs_mtx);
            while (!m_stop) { // 锁有屏障作用，无需担心m_stop多线程访问的问题。
                SpinLock sl(&m_thread_safe_sl);
                while (!m_mapSubscribedEvents.empty()) {
                    auto min = m_mapSubscribedEvents.begin();
                    if (min->first > CommonUtils::GetCurrentTime()) {
                        break;
                    }

                    (*(min->second.callback))(min->second.ctx);
                    EventId evId(min->first, min->second.callback);
                    m_mapEventsEntry.erase(evId);
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
