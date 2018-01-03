/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "rsys/syncint.h"
#include "rsys/blockinterrupts.h"

using namespace Executor;

#if defined(WIN32)

#undef store    /* namespace pollution from db.h */
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

using namespace std::literals::chrono_literals;

namespace
{
    using clock = std::chrono::steady_clock;
    std::mutex mutex;
    std::condition_variable cond;
    std::condition_variable wake_cond;
    clock::time_point last_interrupt;
    clock::time_point scheduled_interrupt;
    std::thread timer_thread;
}

int Executor::syncint_init(void)
{
    std::thread([]() {
        std::unique_lock<std::mutex> lock(mutex);
        for(;;)
        {
            if(scheduled_interrupt > last_interrupt)
            {
                auto status = cond.wait_until(lock, scheduled_interrupt);
                if(status == std::cv_status::timeout)
                {
                    last_interrupt = scheduled_interrupt;

                    interrupt_generate(M68K_TIMER_PRIORITY);
                    wake_cond.notify_all();
                }
            }
            else
            {
                cond.wait(lock);
            }
        }
    }).detach();
    return true;
}

void Executor::syncint_wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    wake_cond.wait_for(lock, 1s, []() { return INTERRUPT_PENDING(); });
}

void Executor::syncint_post(std::chrono::microseconds usecs, bool fromLast)
{
    std::unique_lock<std::mutex> lock(mutex);

    auto time = (fromLast ? last_interrupt : clock::now()) + usecs;

    if(time <= last_interrupt)
        last_interrupt = clock::time_point();
    if(scheduled_interrupt <= last_interrupt || time < scheduled_interrupt)
        scheduled_interrupt = time;
    
    cond.notify_one();
}
#else

#include <chrono>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

static void
handle_itimer_tick(int n)
{
    interrupt_generate(M68K_TIMER_PRIORITY);
}

int Executor::syncint_init(void)
{
    struct sigaction s;

    s.sa_handler = handle_itimer_tick;
    sigemptyset(&s.sa_mask);
    s.sa_flags = 0;
    return (sigaction(SIGALRM, &s, NULL) == 0);
}

/* Posting a delay of 0 will clear any pending interrupt. */
void Executor::syncint_post(std::chrono::microseconds usecs, bool fromLast)
{
    struct itimerval t;

    t.it_value.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(usecs).count();
    t.it_value.tv_usec = usecs.count() % 1000000;
    t.it_interval.tv_sec = 0;
    t.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &t, NULL);
}

void Executor::syncint_wait()
{
    sigset_t zero_mask;
    sigemptyset(&zero_mask);
    sigsuspend(&zero_mask);
}

#endif
