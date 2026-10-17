// com_m_audio_FW1814Device::MapChannelAndFunctionBlock
// Entry: 0000de84
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)
    */

undefined8 __thiscall
com_m_audio_FW1814Device::MapChannelAndFunctionBlock
          (com_m_audio_FW1814Device *this,FunctionBlockMapping *param_1)

{
  undefined8 uVar1;
  long lVar2;
  SelectMap1814 *pSVar3;
  undefined8 *puVar4;
  SelectMap1814 *pSVar5;
  SelectMap1814 local_128 [80];
  undefined8 local_d8;
  undefined8 local_d0;
  undefined8 local_c8;
  undefined8 local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  undefined8 local_a8;
  undefined8 local_a0;
  undefined8 local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 local_80;
  undefined8 local_78;
  undefined8 local_70;
  SelectMap1814 local_68 [86];
  FunctionBlockMapping local_12;
  FunctionBlockMapping local_11;
  
  local_11 = param_1[4];
  local_12 = param_1[5];
  switch(*(undefined4 *)param_1) {
  case 0:
    pSVar3 = local_128;
    puVar4 = &MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)::C_117;
    goto LAB_0000df04;
  case 1:
    local_70 = DAT_0002f948;
    local_78 = DAT_0002f940;
    local_80 = DAT_0002f938;
    local_88 = DAT_0002f930;
    local_90 = DAT_0002f928;
    local_98 = MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)::C_115;
    pSVar3 = (SelectMap1814 *)&local_98;
    goto LAB_0000df0e;
  case 2:
    pSVar3 = local_68;
    puVar4 = &MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)::C_114;
LAB_0000df04:
    pSVar5 = pSVar3;
    for (lVar2 = 10; lVar2 != 0; lVar2 = lVar2 + -1) {
      *(undefined8 *)pSVar5 = *puVar4;
      puVar4 = puVar4 + 1;
      pSVar5 = pSVar5 + 8;
    }
    goto LAB_0000df0e;
  case 3:
    local_11 = (FunctionBlockMapping)0x4;
    break;
  case 4:
    local_11 = (FunctionBlockMapping)0x4;
    if (((byte)param_1[8] & 4) == 0) {
      local_12 = (FunctionBlockMapping)0x0;
    }
    else {
      local_12 = (FunctionBlockMapping)0x1;
    }
    break;
  case 5:
    local_11 = local_12;
    break;
  case 6:
    local_a0 = DAT_0002f988;
    local_a8 = DAT_0002f980;
    local_b0 = DAT_0002f978;
    local_b8 = DAT_0002f970;
    local_c0 = DAT_0002f968;
    local_c8 = DAT_0002f960;
    local_d0 = DAT_0002f958;
    local_d8 = MapChannelAndFunctionBlock(m_audio_b_FWAVCAudioDevice::FunctionBlockMapping&)::C_116;
    pSVar3 = (SelectMap1814 *)&local_d8;
LAB_0000df0e:
    uVar1 = SelectFunctionBlock(this,(uchar *)&local_12,pSVar3,(uchar *)&local_11);
    goto LAB_0000e023;
  case 7:
    local_11 = (FunctionBlockMapping)0x9;
    break;
  case 8:
    if (local_12 == (FunctionBlockMapping)0x0) {
      local_11 = (FunctionBlockMapping)0x5;
    }
    else {
      local_11 = (FunctionBlockMapping)0x6;
    }
    break;
  case 9:
    uVar1 = 0xe00002c2;
    goto LAB_0000e023;
  }
  uVar1 = 0;
LAB_0000e023:
  param_1[5] = local_12;
  param_1[4] = local_11;
  return uVar1;
}
