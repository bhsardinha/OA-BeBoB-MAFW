// OpenAudio Glue driver - real-time worker threads for the per-channel DSP (GLUE_PLAN_v1.md "Dual": in an aggregate the HAL calls
// every Glue device on one IO thread; 32 channels are ~86 % of one core on the build PC). run() splits a job into parts that the
// workers and the calling IO thread take in any order and returns when all are done. Each part is one channel with its own state,
// so the result is identical to running the parts one after another. Mach semaphores only; no locks, no allocation in run().
#pragma once
#include <atomic>
#include <mach/mach.h>
#include <pthread.h>

namespace gluedrv {

class Workers {
public:
    typedef void (*PartFn)(void *ctx, int part);
    bool start(int threads);          // 0 = run everything on the caller
    void stop();
    void run(int parts, PartFn fn, void *ctx);
    int threads() const { return fCount; }
private:
    static void *Main(void *arg);
    void loop(int index);
    void doParts();
    static const int kMax = 7;
    int fCount = 0;
    pthread_t fThread[kMax] = {};
    semaphore_t fGo[kMax] = {};
    semaphore_t fDone = 0;
    std::atomic<bool> fQuit{ false };
    // the current job
    PartFn fFn = nullptr;
    void *fCtx = nullptr;
    int fParts = 0;
    std::atomic<int> fNext{ 0 };
    std::atomic<bool> fBusy{ false };   // a second IO thread (another Glue device) does its own parts instead of waiting
};

} // namespace gluedrv
