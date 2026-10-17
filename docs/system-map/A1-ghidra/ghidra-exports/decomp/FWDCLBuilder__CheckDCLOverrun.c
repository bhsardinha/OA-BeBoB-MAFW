// m_audio_b_FWDCLBuilder::CheckDCLOverrun
// Entry: 000067b0  Size: 140 bytes
// Prototype: undefined CheckDCLOverrun(m_audio_b_FWDCLBuilder * this, IOFWLocalIsochPort * param_1)


/* m_audio_b_FWDCLBuilder::CheckDCLOverrun(IOFWLocalIsochPort*) */

void __thiscall
m_audio_b_FWDCLBuilder::CheckDCLOverrun(m_audio_b_FWDCLBuilder *this,IOFWLocalIsochPort *param_1)

{
  uint uVar1;
  uint uVar2;
  
  ModifyJumpLabel(this,param_1,*(uint *)(this + 0x1a4),*(uint *)(this + 0x1a0));
  ModifyJumpLabel(this,param_1,*(uint *)(this + 0x1a8),*(uint *)(this + 0x1ac));
  uVar1 = *(uint *)(this + 0x1a0);
  uVar2 = *(int *)(this + 0x1a8) + 1;
  if (uVar1 <= uVar2) {
    uVar2 = 0;
  }
  *(uint *)(this + 0x1a8) = uVar2;
  uVar2 = *(int *)(this + 0x1ac) + 1;
  if (uVar1 <= uVar2) {
    uVar2 = 0;
  }
  *(uint *)(this + 0x1ac) = uVar2;
  uVar2 = *(int *)(this + 0x1a4) + 1;
  if (uVar1 <= uVar2) {
    uVar2 = 0;
  }
  *(uint *)(this + 0x1a4) = uVar2;
  return;
}


