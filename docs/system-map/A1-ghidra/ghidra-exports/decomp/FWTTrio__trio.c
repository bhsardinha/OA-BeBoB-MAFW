// m_audio_b_FWTTrio::trio
// Entry: 0001ec2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWTTrio::trio(unsigned int, unsigned int, OSObject*) */

OSObject * m_audio_b_FWTTrio::trio(uint param_1,uint param_2,OSObject *param_3)

{
  OSObject *this;
  undefined4 in_register_00000034;
  
  this = OSObject::operator_new
                   ((OSObject *)&segment_command_00000020,CONCAT44(in_register_00000034,param_2));
  if (this != (OSObject *)0x0) {
    OSObject::OSObject(this,(OSMetaClass *)&gMetaClass);
    *(undefined ***)this = &PTR__m_audio_b_FWTTrio_00047dd0;
    OSMetaClass::instanceConstructed();
    (**(code **)(*(long *)this + 0x88))(this);
    *(uint *)(this + 0xc) = param_1;
    *(uint *)(this + 0x10) = param_2;
    *(OSObject **)(this + 0x18) = param_3;
  }
  return this;
}
