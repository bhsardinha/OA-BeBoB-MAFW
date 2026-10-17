// __static_initialization_and_destruction_0
// Entry: 00008654
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWBaseArrayProp::gMetaClass,"m_audio_b_FWBaseArrayProp",
                 (OSMetaClass *)&m_audio_b_FWBaseProp::gMetaClass,0x60);
      m_audio_b_FWBaseArrayProp::gMetaClass = (undefined *)&PTR__MetaClass_000346c0;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWArrayProp::gMetaClass,"m_audio_b_FWArrayProp",
                 (OSMetaClass *)&m_audio_b_FWBaseArrayProp::gMetaClass,0x60);
      m_audio_b_FWArrayProp::gMetaClass = (undefined *)&PTR__MetaClass_000347b0;
    }
    return;
  }
  m_audio_b_FWArrayProp::gMetaClass = (undefined *)&PTR__MetaClass_000347b0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWArrayProp::gMetaClass);
  m_audio_b_FWBaseArrayProp::gMetaClass = (undefined *)&PTR__MetaClass_000346c0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWBaseArrayProp::gMetaClass);
  return;
}
