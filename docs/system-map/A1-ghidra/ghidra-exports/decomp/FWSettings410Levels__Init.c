// FWSettings410Levels::Init
// Entry: 0001437a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings410Levels::Init(m_audio_b_FWAVCAudioDevice*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*, m_audio_FWb::BaseChannelGroup*,
   m_audio_FWb::BaseChannelGroup*) */

int __thiscall
FWSettings410Levels::Init
          (FWSettings410Levels *this,m_audio_b_FWAVCAudioDevice *param_1,BaseChannelGroup *param_2,
          BaseChannelGroup *param_3,BaseChannelGroup *param_4,BaseChannelGroup *param_5,
          BaseChannelGroup *param_6)

{
  int iVar1;
  undefined8 *puVar2;
  
  iVar1 = (*(code *)PTR_Init_00048e88)(this);
  if (iVar1 == 0) {
    if (*(void **)(this + 0xa38) != (void *)0x0) {
      operator_delete(*(void **)(this + 0xa38));
    }
    if (this == (FWSettings410Levels *)0x0) {
      puVar2 = (undefined8 *)0x0;
    }
    else {
      puVar2 = operator_new(0x20);
      if (puVar2 != (undefined8 *)0x0) {
        *puVar2 = &PTR_Call_0003c4c0;
        puVar2[1] = this;
        puVar2[2] = 0x79;
        puVar2[3] = 0;
      }
    }
    *(undefined8 **)(this + 0xa38) = puVar2;
  }
  return iVar1;
}
