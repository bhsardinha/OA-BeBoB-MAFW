// Kernel module descriptor (standard KMOD_EXPLICIT_DECL boilerplate; the IOKit libkmodc++ start/stop run static ctors).
#include <mach/mach_types.h>
#include <libkern/libkern.h>

extern "C" {
extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);

__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(com.openaudio.OpenAudioBeBoBMAFW, "0.2.1", _start, _stop)
__private_extern__ kmod_start_func_t *_realmain = 0;
__private_extern__ kmod_stop_func_t  *_antimain = 0;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
}
