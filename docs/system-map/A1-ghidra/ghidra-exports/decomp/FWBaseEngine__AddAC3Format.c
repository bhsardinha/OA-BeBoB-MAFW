// m_audio_b_FWBaseEngine::AddAC3Format
// Entry: 000039a0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* m_audio_b_FWBaseEngine::AddAC3Format(IOAudioStream*) */

void __thiscall
m_audio_b_FWBaseEngine::AddAC3Format(m_audio_b_FWBaseEngine *this,IOAudioStream *param_1)

{
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined1 local_44;
  undefined1 local_43;
  undefined1 local_42;
  undefined1 local_41;
  undefined1 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  
  local_38 = 1;
  local_34 = 0;
  local_30 = 0x600;
  local_2c = 0x1800;
  local_50 = 2;
  local_4c = 0x63616333;
  local_48 = 0x73696e74;
  local_44 = 0x10;
  local_43 = 0x10;
  local_42 = 0;
  local_41 = 1;
  local_40 = 0;
  local_3c = 0;
  local_58 = 48000;
  local_54 = 0;
  (**(code **)(*(long *)param_1 + 0x868))(param_1,&local_50,&local_38,&local_58,&local_58,0,0);
  local_58 = 0xac44;
  (**(code **)(*(long *)param_1 + 0x868))(param_1,&local_50,&local_38,&local_58,&local_58,0,0);
  local_58 = 96000;
  (**(code **)(*(long *)param_1 + 0x868))(param_1,&local_50,&local_38,&local_58,&local_58,0,0);
  return;
}
