// __static_initialization_and_destruction_0
// Entry: 000275c4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 != 0) {
    if (param_1 == 1) {
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWDCLProgram::gMetaClass,"m_audio_b_FWDCLProgram",
                 (OSMetaClass *)PTR_gMetaClass_00030088,0x508);
      _gMetaClass = &PTR__MetaClass_0004a4d0;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWDCLOutputProgram::gMetaClass,
                 "m_audio_b_FWDCLOutputProgram",(OSMetaClass *)&m_audio_b_FWDCLProgram::gMetaClass,
                 0x620);
      _gMetaClass = &PTR__MetaClass_0004a5c0;
      OSMetaClass::OSMetaClass
                ((OSMetaClass *)&m_audio_b_FWDCLInputProgram::gMetaClass,
                 "m_audio_b_FWDCLInputProgram",(OSMetaClass *)&m_audio_b_FWDCLProgram::gMetaClass,
                 0x518);
      _gMetaClass = &PTR__MetaClass_0004a6b0;
    }
    return;
  }
  _gMetaClass = &PTR__MetaClass_0004a6b0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWDCLInputProgram::gMetaClass);
  _gMetaClass = &PTR__MetaClass_0004a5c0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWDCLOutputProgram::gMetaClass);
  _gMetaClass = &PTR__MetaClass_0004a4d0;
  OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWDCLProgram::gMetaClass);
  return;
}
