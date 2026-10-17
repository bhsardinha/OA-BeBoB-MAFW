// __static_initialization_and_destruction_0
// Entry: 00006122
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWBaseEngine::gMetaClass,"m_audio_b_FWBaseEngine",
                 (OSMetaClass *)&m_audio_b_FWDiscreteEngine::gMetaClass,0x350);
      m_audio_b_FWBaseEngine::gMetaClass = (undefined *)&PTR__MetaClass_00031ce0;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWEngineUserClient::gMetaClass,
                 "m_audio_b_FWEngineUserClient",(OSMetaClass *)PTR_gMetaClass_00030060,0x230);
      m_audio_b_FWEngineUserClient::gMetaClass = (undefined *)&PTR__MetaClass_00031dd0;
    }
    return;
  }
  m_audio_b_FWEngineUserClient::gMetaClass = (undefined *)&PTR__MetaClass_00031dd0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWEngineUserClient::gMetaClass);
  m_audio_b_FWBaseEngine::gMetaClass = (undefined *)&PTR__MetaClass_00031ce0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWBaseEngine::gMetaClass);
  return;
}
