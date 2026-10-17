// m_audio_b_FWDCLProgram::SetupCIP
// Entry: 00027102  Size: 468 bytes
// Prototype: undefined SetupCIP(m_audio_b_FWDCLProgram * this, uint param_1, SYTInterval param_2, SYTIncrement param_3, FDFValue param_4)


/* m_audio_b_FWDCLProgram::SetupCIP(unsigned int, SYTInterval, SYTIncrement, FDFValue) */

void __thiscall
m_audio_b_FWDCLProgram::SetupCIP
          (m_audio_b_FWDCLProgram *this,int param_1,undefined4 param_3,undefined4 param_4,
          undefined4 param_5)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  ushort local_20;
  ushort local_1e;
  ushort local_1c [2];
  
  iVar3 = (int)(0x3e80000 / (ulong)(uint)(param_1 * 4));
  *(int *)(this + 0x4dc) = iVar3 + -2;
  *(int *)(this + 0x4e0) = iVar3 + 2;
  this[0x4d0] = (m_audio_b_FWDCLProgram)0x0;
  *(undefined4 *)(this + 0x4ac) = param_3;
  *(undefined4 *)(this + 0x4a8) = param_4;
  *(undefined4 *)(this + 0x4a4) = param_5;
  *(int *)(this + 0x4a0) = param_1;
  IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0x78),local_1c,&local_1e);
  *(uint *)(this + 0x498) =
       (local_20 & 0x3f) << 0x18 | (*(int *)(this + 0x68) + *(int *)(this + 100)) * 0x10000;
  *(undefined4 *)(this + 0x49c) = 0x90000000;
  switch(param_5) {
  case 0:
    *(undefined4 *)(this + 0x4c0) = 1;
    *(undefined4 *)(this + 0x4c4) = 2;
    *(undefined4 *)(this + 0x4b0) = 1;
    *(undefined4 *)(this + 0x4b4) = 1;
    iVar3 = 2;
    break;
  case 1:
  case 3:
  case 5:
    *(undefined4 *)(this + 0x4c0) = 199;
    *(undefined4 *)(this + 0x4c4) = 0x280;
    *(undefined4 *)(this + 0x4b0) = 0x153;
    *(undefined4 *)(this + 0x4b4) = 0x1b9;
    iVar4 = 199;
    uVar1 = 0x153;
    iVar3 = 0x280;
    goto LAB_0002727e;
  case 2:
  case 4:
  case 6:
    *(undefined4 *)(this + 0x4c0) = 1;
    *(undefined4 *)(this + 0x4c4) = 4;
    *(undefined4 *)(this + 0x4b0) = 1;
    *(undefined4 *)(this + 0x4b4) = 1;
    iVar3 = 4;
    break;
  default:
    iVar3 = *(int *)(this + 0x4c4);
    uVar1 = *(undefined4 *)(this + 0x4b0);
    iVar4 = *(int *)(this + 0x4c0);
    goto LAB_0002727e;
  }
  uVar1 = 1;
  iVar4 = 1;
LAB_0002727e:
  *(int *)(this + 0x4bc) = iVar4;
  *(undefined4 *)(this + 0x4b8) = uVar1;
  uVar5 = (iVar3 - iVar4) * *(int *)(this + 0x4ac);
  uVar2 = (uint)(iVar3 << 0xb) / uVar5;
  uVar6 = uVar2 + 1;
  if ((uint)(iVar3 << 0xb) % uVar5 == 0) {
    uVar6 = uVar2;
  }
  *(uint *)(this + 0x4c8) = uVar6;
  uVar2 = (uint)(iVar3 << 0xc) / uVar5;
  *(uint *)(this + 0x4cc) = uVar2;
  if ((uint)(iVar3 << 0xc) % uVar5 != 0) {
    *(uint *)(this + 0x4cc) = uVar2 + 1;
  }
  return;
}


