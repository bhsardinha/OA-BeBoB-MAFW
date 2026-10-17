// com_m_audio_FWMetaNub::message
// Entry: 0001b9d6  Size: 173 bytes
// Prototype: undefined message(com_m_audio_FWMetaNub * this, uint param_1, IOService * param_2, void * param_3)


/* com_m_audio_FWMetaNub::message(unsigned int, IOService*, void*) */

void __thiscall
com_m_audio_FWMetaNub::message
          (com_m_audio_FWMetaNub *this,uint param_1,IOService *param_2,void *param_3)

{
  ushort *puVar1;
  ushort *puVar2;
  uint *puVar3;
  ushort local_36;
  ushort local_34 [3];
  ushort local_2e;
  ushort local_2c [2];
  
  if (param_1 == 0xe0000020) {
    this[0xf2] = this[0xf1];
    puVar3 = *(uint **)(this + 0xe0);
    if (puVar3 == (uint *)0x0) goto LAB_0001ba49;
    puVar2 = local_34;
    puVar1 = &local_36;
  }
  else {
    if ((param_1 != 0xe0000030) || (puVar3 = *(uint **)(this + 0xe0), puVar3 == (uint *)0x0))
    goto LAB_0001ba49;
    puVar2 = local_2c;
    puVar1 = &local_2e;
  }
  IOFireWireNub::getNodeIDGeneration(puVar3,puVar2,puVar1);
LAB_0001ba49:
  (**(code **)(*(long *)this + 0x740))(this,param_1,0,0);
  (**(code **)(PTR_vtable_000300e0 + 0x740))(this,param_1,param_2,param_3);
  return;
}


