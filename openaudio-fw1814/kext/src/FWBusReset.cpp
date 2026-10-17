#include "FWBusReset.h"

bool FWRunOnNewThread(void (*fn)(void *), void *arg)
{
    return IOCreateThread(fn, arg) != 0;
}
