// m_audio_b_FWDCLBuilder::ModifyJumpLabel
// Entry: 00006734  Size: 123 bytes
// Prototype: undefined ModifyJumpLabel(m_audio_b_FWDCLBuilder * this, IOFWLocalIsochPort * param_1, uint param_2, uint param_3)


/* m_audio_b_FWDCLBuilder::ModifyJumpLabel(IOFWLocalIsochPort*, unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWDCLBuilder::ModifyJumpLabel
          (m_audio_b_FWDCLBuilder *this,IOFWLocalIsochPort *param_1,uint param_2,uint param_3)

{
  long *plVar1;
  undefined8 *puVar2;
  long local_10;
  
  plVar1 = *(long **)(this + 0x138);
  if (-1 < (int)param_2 && param_2 < *(uint *)(this + 0x140)) {
    plVar1 = plVar1 + (int)param_2;
  }
  local_10 = *plVar1;
  puVar2 = *(undefined8 **)(this + 0x28);
  if (-1 < (int)param_3 && param_3 < *(uint *)(this + 0x30)) {
    puVar2 = puVar2 + (int)param_3;
  }
  *(undefined8 *)(local_10 + 0x18) = *puVar2;
  if (param_1 != (IOFWLocalIsochPort *)0x0) {
    (**(code **)(*(long *)param_1 + 0x158))(param_1,2,&local_10,1);
  }
  return;
}


