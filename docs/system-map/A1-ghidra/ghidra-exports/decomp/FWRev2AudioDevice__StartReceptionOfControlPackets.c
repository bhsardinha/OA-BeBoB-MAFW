// FWRev2AudioDevice::StartReceptionOfControlPackets
// Entry: 00013138
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::StartReceptionOfControlPackets() */

undefined1 FWRev2AudioDevice::StartReceptionOfControlPackets(void)

{
  long lVar1;
  char cVar2;
  undefined1 uVar3;
  undefined4 uVar4;
  m_audio_b_FWHSCI *this;
  undefined8 *puVar5;
  IOWorkLoop *pIVar6;
  ulong in_RSI;
  long *in_RDI;
  
  uVar4 = (**(code **)(*in_RDI + 0xba0))();
  lVar1 = in_RDI[0x1c];
  this = OSObject::operator_new((OSObject *)&section_000000b8.addr,in_RSI);
  if (this == (m_audio_b_FWHSCI *)0x0) {
LAB_00013222:
    in_RDI[0x59] = (long)this;
    if (this != (m_audio_b_FWHSCI *)0x0) {
      pIVar6 = (IOWorkLoop *)(**(code **)(*in_RDI + 0x688))();
      uVar3 = m_audio_b_FWHSCI::Start((m_audio_b_FWHSCI *)in_RDI[0x59],pIVar6);
      return uVar3;
    }
  }
  else {
    m_audio_b_FWHSCI::m_audio_b_FWHSCI(this);
    cVar2 = (**(code **)(*(long *)this + 0x88))(this);
    if (cVar2 != '\0') {
      *(long *)(this + 0x10) = lVar1;
      *(undefined4 *)(this + 0xc0) = uVar4;
      if (*(void **)(this + 0xb8) != (void *)0x0) {
        operator_delete(*(void **)(this + 0xb8));
      }
      if (in_RDI == (long *)0x0) {
        *(undefined8 *)(this + 0xb8) = 0;
      }
      else {
        puVar5 = operator_new(0x20);
        if (puVar5 != (undefined8 *)0x0) {
          *puVar5 = &PTR_Call_00039f40;
          puVar5[1] = in_RDI;
          puVar5[2] = 0xbb9;
          puVar5[3] = 0;
          *(undefined8 **)(this + 0xb8) = puVar5;
          this[0xd0] = (m_audio_b_FWHSCI)0x1;
          goto LAB_00013222;
        }
        *(undefined8 *)(this + 0xb8) = 0;
      }
    }
    (**(code **)(*(long *)this + 0x28))(this);
    in_RDI[0x59] = 0;
  }
  return 0;
}
