// __static_initialization_and_destruction_0
// Entry: 00001a0c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWAsyncAVCCommand::gMetaClass,
                 "m_audio_b_FWAsyncAVCCommand",(OSMetaClass *)PTR_gMetaClass_00030068,0xa8);
      m_audio_b_FWAsyncAVCCommand::gMetaClass = (undefined *)&PTR__MetaClass_000303b0;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWAVCUnit::gMetaClass,"m_audio_b_FWAVCUnit",
                 (OSMetaClass *)PTR_gMetaClass_00030058,0xf8);
      m_audio_b_FWAVCUnit::gMetaClass = (undefined *)&PTR__MetaClass_000304a0;
    }
    return;
  }
  m_audio_b_FWAVCUnit::gMetaClass = (undefined *)&PTR__MetaClass_000304a0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWAVCUnit::gMetaClass);
  m_audio_b_FWAsyncAVCCommand::gMetaClass = (undefined *)&PTR__MetaClass_000303b0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWAsyncAVCCommand::gMetaClass);
  return;
}
