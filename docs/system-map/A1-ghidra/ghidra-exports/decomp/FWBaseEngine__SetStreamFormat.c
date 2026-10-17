// m_audio_b_FWBaseEngine::SetStreamFormat
// Entry: 00004192
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::SetStreamFormat(IOAudioStream*, m_audio_b_FWAudioDevice::DeviceChannels&,
   unsigned int) */

undefined8
m_audio_b_FWBaseEngine::SetStreamFormat(IOAudioStream *param_1,DeviceChannels *param_2,uint param_3)

{
  int iVar1;
  char cVar2;
  undefined4 *puVar3;
  undefined8 uVar4;
  undefined4 in_register_00000014;
  long lVar5;
  undefined8 *puVar6;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 local_34;
  undefined1 local_33;
  undefined1 local_32;
  undefined1 local_31;
  undefined1 local_30;
  undefined4 local_2c;
  
  lVar5 = CONCAT44(in_register_00000014,param_3);
  if (param_2 == (DeviceChannels *)0x0) {
    uVar4 = 0xe00002c2;
  }
  else {
    iVar1 = *(int *)(param_2 + 0xac);
    (**(code **)(*(long *)param_2 + 0xa68))(param_2);
    if (iVar1 == 0) {
      puVar3 = (undefined4 *)(lVar5 + 0xc);
    }
    else {
      puVar3 = (undefined4 *)(lVar5 + 4);
    }
    local_40 = *puVar3;
    local_3c = 0x6c70636d;
    local_38 = 0x73696e74;
    local_34 = 0x18;
    local_33 = 0x20;
    local_32 = 0;
    local_31 = 0;
    local_30 = 1;
    local_2c = 0;
    if (iVar1 == 0) {
      puVar6 = (undefined8 *)(lVar5 + 0x38);
    }
    else {
      puVar6 = (undefined8 *)(lVar5 + 0x30);
    }
    ReplicateFormatForEachSampleRate
              ((m_audio_b_FWBaseEngine *)param_1,(IOAudioStream *)param_2,
               (_IOAudioStreamFormat *)&local_40,(_IOAudioSampleRate *)*puVar6);
    if (((iVar1 == 0) && (param_1[0x2d0] == (IOAudioStream)0x0)) &&
       (*(long **)(param_1 + 0x138) != (long *)0x0)) {
      cVar2 = (**(code **)(**(long **)(param_1 + 0x138) + 0xaa8))();
      if (cVar2 != '\0') {
        AddAC3Format((m_audio_b_FWBaseEngine *)param_1,(IOAudioStream *)param_2);
      }
    }
    uVar4 = (**(code **)(*(long *)param_2 + 0xa38))(param_2,&local_40,0);
  }
  return uVar4;
}
