// com_m_audio_FW410Device::MapChannelAndFunctionBlock
// Entry: 00014600
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)
    */

undefined8 __thiscall
com_m_audio_FW410Device::MapChannelAndFunctionBlock
          (com_m_audio_FW410Device *this,FunctionBlockMapping *param_1)

{
  undefined8 uVar1;
  FunctionBlockMapping FVar2;
  FunctionBlockMapping FVar3;
  FunctionBlockMapping *pFVar4;
  int iVar5;
  
  FVar2 = param_1[4];
  FVar3 = param_1[5];
  switch(*(undefined4 *)param_1) {
  case 0:
    pFVar4 = (FunctionBlockMapping *)&DAT_0002fa58;
    for (iVar5 = 0; iVar5 != 4; iVar5 = iVar5 + 1) {
      if ((*(uint *)(pFVar4 + -8) <= (uint)(byte)FVar3) &&
         ((uint)(byte)FVar3 <= *(uint *)(pFVar4 + -4))) {
        FVar2 = *pFVar4;
        if ((iVar5 != 0) &&
           (FVar2 != *(FunctionBlockMapping *)(&DAT_0002fa58 + (ulong)(iVar5 - 1) * 0xc))) {
          FVar3 = (FunctionBlockMapping)(((char)FVar3 + 1U) - (char)*(uint *)(pFVar4 + -8));
        }
        goto switchD_00014624_default;
      }
      pFVar4 = pFVar4 + 0xc;
    }
  case 8:
  case 9:
    uVar1 = 0xe00002c2;
    break;
  case 1:
    uVar1 = 0;
    FVar2 = (FunctionBlockMapping)0xf;
    break;
  case 2:
    if ((byte)((char)FVar3 - 1U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xa;
      break;
    }
    if ((byte)((char)FVar3 - 3U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xb;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 2);
      break;
    }
    if ((byte)((char)FVar3 - 5U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xc;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 4);
      break;
    }
    if ((byte)((char)FVar3 - 7U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xd;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 6);
      break;
    }
    if ((byte)((char)FVar3 - 9U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xe;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 8);
      break;
    }
    goto LAB_000146b8;
  case 3:
    if ((byte)((char)FVar3 - 1U) < 8) {
switchD_00014663_caseD_2:
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x1;
      break;
    }
    if ((byte)((char)FVar3 - 9U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x2;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 8);
      break;
    }
    goto LAB_000146b8;
  case 4:
    switch(*(uint *)(param_1 + 8) & 7) {
    case 0:
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x1;
      FVar3 = (FunctionBlockMapping)0x0;
      break;
    case 1:
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x1;
      FVar3 = (FunctionBlockMapping)0x2;
      break;
    default:
      goto switchD_00014663_caseD_2;
    case 4:
      uVar1 = 0;
      FVar3 = (FunctionBlockMapping)0x1;
      FVar2 = (FunctionBlockMapping)0x1;
      break;
    case 5:
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x1;
      FVar3 = (FunctionBlockMapping)0x3;
    }
    break;
  case 5:
    FVar2 = (FunctionBlockMapping)((char)FVar3 + 1);
  default:
switchD_00014624_default:
    uVar1 = 0;
    break;
  case 6:
    if ((byte)((char)FVar3 - 1U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x3;
      break;
    }
    if ((byte)((char)FVar3 - 3U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x4;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 2);
      break;
    }
LAB_000146b8:
    uVar1 = 0xe00002c2;
    break;
  case 7:
    uVar1 = 0;
    FVar2 = (FunctionBlockMapping)0x9;
  }
  param_1[5] = FVar3;
  param_1[4] = FVar2;
  return uVar1;
}
