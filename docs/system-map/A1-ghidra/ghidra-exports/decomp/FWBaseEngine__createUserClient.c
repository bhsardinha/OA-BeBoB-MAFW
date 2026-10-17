// m_audio_b_FWBaseEngine::createUserClient
// Entry: 00005fa2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWBaseEngine::createUserClient(task*, void*, unsigned int, IOAudioEngineUserClient**)
    */

undefined8 __thiscall
m_audio_b_FWBaseEngine::createUserClient
          (m_audio_b_FWBaseEngine *this,task *param_1,void *param_2,uint param_3,
          IOAudioEngineUserClient **param_4)

{
  char cVar1;
  IOAudioEngineUserClient *this_00;
  undefined8 uVar2;
  
  this_00 = OSObject::operator_new((OSObject *)&section_000001f0.flags,(ulong)param_1);
  if (this_00 != (IOAudioEngineUserClient *)0x0) {
    IOAudioEngineUserClient::IOAudioEngineUserClient
              (this_00,(OSMetaClass *)&m_audio_b_FWEngineUserClient::gMetaClass);
    *(undefined ***)this_00 = &PTR__m_audio_b_FWEngineUserClient_00031ec0;
    OSMetaClass::instanceConstructed();
    cVar1 = (**(code **)(*(long *)this_00 + 0xaa0))(this_00,this,param_1,param_2,param_3);
    if (cVar1 == '\0') {
      (**(code **)(*(long *)this_00 + 0x28))(this_00);
      this_00 = (IOAudioEngineUserClient *)0x0;
    }
  }
  *param_4 = this_00;
  uVar2 = 0xe00002bd;
  if (this_00 != (IOAudioEngineUserClient *)0x0) {
    uVar2 = 0;
  }
  return uVar2;
}
