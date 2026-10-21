// FWAudioDeviceChain -- umbrella for the base device chain (one module per reference class):
//   IOAudioDevice -> FWDiscreteDevice -> FWAudioDevice -> FWAVCAudioDevice -> FWRev1AudioDevice | FWRev2AudioDevice -> <model>
// The reference names are m_audio_b_FW{Discrete,Audio,AVCAudio}Device / FWRev1AudioDevice / FWRev2AudioDevice (Q16 Correction 2026-10-14,
// Q-AP0 §2). Model classes live in models/<Model>/ and include the revision header they derive from; shared code includes only this file.
// Every free() is NULL-safe (10.13.6 operator delete(NULL) panics, Q-SDK5 obs. 8-10).
#ifndef OA_FWAUDIODEVICECHAIN_H
#define OA_FWAUDIODEVICECHAIN_H

#include "FWAudioDevice.h"
#include "FWAVCAudioDevice.h"
#include "FWRev1AudioDevice.h"
#include "FWRev2AudioDevice.h"

#endif
