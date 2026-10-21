// com_m_audio_FWAudiophileDevice::MapChannelAndFunctionBlock
// Entry: 0000b0e2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)
    */

undefined8 __thiscall
com_m_audio_FWAudiophileDevice::MapChannelAndFunctionBlock
          (com_m_audio_FWAudiophileDevice *this,FunctionBlockMapping *param_1)

{
  undefined8 uVar1;
  FunctionBlockMapping FVar2;
  FunctionBlockMapping FVar3;
  FunctionBlockMapping *pFVar4;
  int iVar5;
  ulong uVar6;
  
  FVar2 = param_1[4];
  FVar3 = param_1[5];
  switch(*(undefined4 *)param_1) {
  case 0:
    pFVar4 = (FunctionBlockMapping *)&DAT_0002f758;
    uVar6 = 0xffffffff;
    while (iVar5 = (int)uVar6, iVar5 != 4) {
      if ((*(uint *)(pFVar4 + -8) <= (uint)(byte)FVar3) &&
         ((uint)(byte)FVar3 <= *(uint *)(pFVar4 + -4))) {
        FVar2 = *pFVar4;
        if ((iVar5 != -1) && (FVar2 != *(FunctionBlockMapping *)(&DAT_0002f758 + uVar6 * 0xc))) {
          FVar3 = (FunctionBlockMapping)(((char)FVar3 + 1U) - (char)*(uint *)(pFVar4 + -8));
        }
        goto switchD_0000b106_default;
      }
      pFVar4 = pFVar4 + 0xc;
      uVar6 = (ulong)(iVar5 + 1);
    }
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
      FVar2 = (FunctionBlockMapping)0xc;
      break;
    }
    if ((byte)((char)FVar3 - 3U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xd;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 2);
      break;
    }
    if ((byte)((char)FVar3 - 5U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0xe;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 4);
      break;
    }
    goto LAB_0000b174;
  case 3:
    if ((byte)((char)FVar3 - 1U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x1;
      break;
    }
    if ((byte)((char)FVar3 - 3U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x2;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 2);
      break;
    }
    if ((byte)((char)FVar3 - 5U) < 2) {
      uVar1 = 0;
      FVar2 = (FunctionBlockMapping)0x3;
      FVar3 = (FunctionBlockMapping)((char)FVar3 - 4);
      break;
    }
LAB_0000b174:
    uVar1 = 0;
    break;
  case 4:
    uVar1 = 0xe00002c7;
    break;
  case 5:
    uVar1 = 0;
    FVar2 = FVar3;
    break;
  case 6:
    if (1 < (byte)((char)FVar3 - 1U)) {
      if ((byte)((char)FVar3 - 3U) < 2) {
        uVar1 = 0;
        FVar2 = (FunctionBlockMapping)0x5;
        FVar3 = (FunctionBlockMapping)((char)FVar3 - 2);
      }
      else {
        uVar1 = 0xe00002c2;
      }
      break;
    }
  case 8:
    uVar1 = 0;
    FVar2 = (FunctionBlockMapping)0x4;
    break;
  case 7:
    uVar1 = 0;
    FVar2 = (FunctionBlockMapping)0xb;
    break;
  default:
switchD_0000b106_default:
    uVar1 = 0;
  }
  param_1[5] = FVar3;
  param_1[4] = FVar2;
  return uVar1;
}
