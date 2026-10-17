// __static_initialization_and_destruction_0
// Entry: 0000fa1a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWNumberProp::gMetaClass,"m_audio_b_FWNumberProp",
                 (OSMetaClass *)&m_audio_b_FWBaseProp::gMetaClass,0x30);
      m_audio_b_FWNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037650;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FW32BitNumberProp::gMetaClass,
                 "m_audio_b_FW32BitNumberProp",(OSMetaClass *)&m_audio_b_FWNumberProp::gMetaClass,
                 0x40);
      m_audio_b_FW32BitNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037740;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FW64BitNumberProp::gMetaClass,
                 "m_audio_b_FW64BitNumberProp",(OSMetaClass *)&m_audio_b_FWNumberProp::gMetaClass,
                 0x48);
      m_audio_b_FW64BitNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037830;
    }
    return;
  }
  m_audio_b_FW64BitNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037830;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FW64BitNumberProp::gMetaClass);
  m_audio_b_FW32BitNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037740;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FW32BitNumberProp::gMetaClass);
  m_audio_b_FWNumberProp::gMetaClass = (undefined *)&PTR__MetaClass_00037650;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWNumberProp::gMetaClass);
  return;
}
