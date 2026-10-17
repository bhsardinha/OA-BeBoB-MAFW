// __static_initialization_and_destruction_0
// Entry: 0000370c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* __static_initialization_and_destruction_0(int, int) */

void __static_initialization_and_destruction_0(int param_1,int param_2)

{
  if (param_1 == 0) {
    m_audio_b_FWAudioDevice::gMetaClass = (undefined *)&PTR__MetaClass_00031070;
    OSMetaClass::~OSMetaClass((OSMetaClass *)&m_audio_b_FWAudioDevice::gMetaClass);
    return;
  }
  if (param_1 == 1) {
    OSMetaClass::OSMetaClass
              ((OSMetaClass *)&m_audio_b_FWAudioDevice::gMetaClass,"m_audio_b_FWAudioDevice",
               (OSMetaClass *)&m_audio_b_FWDiscreteDevice::gMetaClass,0x268);
    m_audio_b_FWAudioDevice::gMetaClass = (undefined *)&PTR__MetaClass_00031070;
    _strlen("DriverCPULoad");
    return;
  }
  return;
}
