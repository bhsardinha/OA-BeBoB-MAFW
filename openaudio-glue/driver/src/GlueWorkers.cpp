// OpenAudio Glue driver - real-time worker threads (see GlueWorkers.h).
#include "GlueWorkers.h"
#include <mach/mach_time.h>
#include <mach/thread_policy.h>

namespace gluedrv {

struct StartArg { Workers *w; int index; };

void *Workers::Main(void *arg)
{
    StartArg a = *static_cast<StartArg *>(arg);
    delete static_cast<StartArg *>(arg);
    pthread_setname_np("OpenAudio Glue DSP");
    // Time-constraint (real-time) policy, as an IO thread has: period about one 128-frame buffer at 44.1 kHz, so the scheduler
    // runs the workers as promptly as the IO thread that waits for them.
    mach_timebase_info_data_t tb; mach_timebase_info(&tb);
    const double ticksPerNs = (double)tb.denom / tb.numer;
    thread_time_constraint_policy_data_t p;
    p.period = (uint32_t)(2900000.0 * ticksPerNs);
    p.computation = (uint32_t)(1500000.0 * ticksPerNs);
    p.constraint = (uint32_t)(2900000.0 * ticksPerNs);
    p.preemptible = 1;
    thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_TIME_CONSTRAINT_POLICY, (thread_policy_t)&p,
                      THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    a.w->loop(a.index);
    return nullptr;
}

bool Workers::start(int threads)
{
    if (threads > kMax) threads = kMax;
    if (threads < 0) threads = 0;
    if (semaphore_create(mach_task_self(), &fDone, SYNC_POLICY_FIFO, 0) != KERN_SUCCESS) return false;
    fQuit.store(false);
    for (int i = 0; i < threads; i++) {
        if (semaphore_create(mach_task_self(), &fGo[i], SYNC_POLICY_FIFO, 0) != KERN_SUCCESS) break;
        StartArg *a = new StartArg{ this, i };
        if (pthread_create(&fThread[i], nullptr, &Main, a) != 0) { delete a; semaphore_destroy(mach_task_self(), fGo[i]); break; }
        fCount = i + 1;
    }
    return true;
}

void Workers::stop()
{
    fQuit.store(true);
    for (int i = 0; i < fCount; i++) semaphore_signal(fGo[i]);
    for (int i = 0; i < fCount; i++) { pthread_join(fThread[i], nullptr); semaphore_destroy(mach_task_self(), fGo[i]); }
    fCount = 0;
    if (fDone) { semaphore_destroy(mach_task_self(), fDone); fDone = 0; }
}

void Workers::doParts()
{
    for (;;) {
        const int i = fNext.fetch_add(1, std::memory_order_acq_rel);
        if (i >= fParts) return;
        fFn(fCtx, i);
    }
}

void Workers::loop(int index)
{
    for (;;) {
        semaphore_wait(fGo[index]);
        if (fQuit.load()) return;
        doParts();
        semaphore_signal(fDone);
    }
}

void Workers::run(int parts, PartFn fn, void *ctx)
{
    if (parts <= 0) return;
    if (fCount == 0 || parts == 1 || fBusy.exchange(true, std::memory_order_acq_rel)) {
        for (int i = 0; i < parts; i++) fn(ctx, i);                     // same order, same result
        return;
    }
    const int helpers = fCount < parts - 1 ? fCount : parts - 1;    // no point waking more workers than spare parts
    fFn = fn; fCtx = ctx; fParts = parts;
    fNext.store(0, std::memory_order_release);
    for (int i = 0; i < helpers; i++) semaphore_signal(fGo[i]);
    doParts();
    for (int i = 0; i < helpers; i++) semaphore_wait(fDone);
    fBusy.store(false, std::memory_order_release);
}

} // namespace gluedrv
