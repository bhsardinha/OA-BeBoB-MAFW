// __static_initialization_and_destruction_0
// Entry: 00024bc4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&FWP2PConnection::gMetaClass,"FWP2PConnection",
                 (OSMetaClass *)PTR_gMetaClass_00030088,0x18);
      FWP2PConnection::gMetaClass = (undefined *)&PTR__MetaClass_00049c30;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWAVCConnectionManager::gMetaClass,
                 "m_audio_b_FWAVCConnectionManager",
                 (OSMetaClass *)&m_audio_b_FWConnectionManager::gMetaClass,0x38);
      m_audio_b_FWAVCConnectionManager::gMetaClass = (undefined *)&PTR__MetaClass_00049d20;
    }
    return;
  }
  m_audio_b_FWAVCConnectionManager::gMetaClass = (undefined *)&PTR__MetaClass_00049d20;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWAVCConnectionManager::gMetaClass);
  FWP2PConnection::gMetaClass = (undefined *)&PTR__MetaClass_00049c30;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&FWP2PConnection::gMetaClass);
  return;
}
