// m_audio_b_FWAudioDevice::MaximumBandwidthBytesSupported
// Entry: 00002234
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWAudioDevice::MaximumBandwidthBytesSupported() */

uint __thiscall
m_audio_b_FWAudioDevice::MaximumBandwidthBytesSupported(m_audio_b_FWAudioDevice *this)

{
  uint uVar1;
  uint uVar2;
  long *plVar3;
  long *plVar4;
  
  if (sAudioDevices == (OSCollection *)0x0) {
    uVar2 = 0x186a;
    plVar3 = (long *)0x0;
  }
  else {
    plVar3 = (long *)OSCollectionIterator::withCollection(sAudioDevices);
    if (plVar3 != (long *)0x0) {
      plVar4 = (long *)(**(code **)(*plVar3 + 0x128))(plVar3);
      if (plVar4 != (long *)0x0) {
        uVar2 = 0x186a;
        do {
          uVar1 = (**(code **)(*plVar4 + 0xb30))();
          if (uVar1 < uVar2) {
            uVar2 = uVar1;
          }
          plVar4 = (long *)(**(code **)(*plVar3 + 0x128))(plVar3);
        } while (plVar4 != (long *)0x0);
        goto LAB_000022ab;
      }
    }
    uVar2 = 0x186a;
  }
LAB_000022ab:
  uVar1 = (**(code **)(*(long *)this + 0xb30))(this);
  if (uVar1 < uVar2) {
    uVar2 = (**(code **)(*(long *)this + 0xb30))(this);
  }
  if (plVar3 != (long *)0x0) {
    (**(code **)(*plVar3 + 0x28))(plVar3);
  }
  return uVar2;
}
