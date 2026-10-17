// __static_initialization_and_destruction_0
// Entry: 00013430
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&FWRev2AudioDevice::gMetaClass,"FWRev2AudioDevice",
                 (OSMetaClass *)&m_audio_b_FWAVCAudioDevice::gMetaClass,0x2d0);
      FWRev2AudioDevice::gMetaClass = (undefined *)&PTR__MetaClass_00039f60;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&com_m_audio_MakeAppleFWAudioGoAway::gMetaClass,
                 "com_m_audio_MakeAppleFWAudioGoAway",(OSMetaClass *)PTR_gMetaClass_00030098,0x88);
      com_m_audio_MakeAppleFWAudioGoAway::gMetaClass = (undefined *)&PTR__MetaClass_0003a050;
    }
    return;
  }
  com_m_audio_MakeAppleFWAudioGoAway::gMetaClass = (undefined *)&PTR__MetaClass_0003a050;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&com_m_audio_MakeAppleFWAudioGoAway::gMetaClass);
  FWRev2AudioDevice::gMetaClass = (undefined *)&PTR__MetaClass_00039f60;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&FWRev2AudioDevice::gMetaClass);
  return;
}
