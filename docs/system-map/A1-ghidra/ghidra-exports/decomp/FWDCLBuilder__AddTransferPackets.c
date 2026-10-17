// m_audio_b_FWDCLBuilder::AddTransferPackets
// Entry: 00006b5a  Size: 113 bytes
// Prototype: undefined AddTransferPackets(m_audio_b_FWDCLBuilder * this, uint param_1, uint param_2)


/* m_audio_b_FWDCLBuilder::AddTransferPackets(unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWDCLBuilder::AddTransferPackets(m_audio_b_FWDCLBuilder *this,uint param_1,uint param_2)

{
  undefined8 *puVar1;
  DCLCommandStruct **local_28;
  uint local_20;
  uint local_1c;
  undefined4 local_18;
  undefined4 local_14;
  
  local_28 = (DCLCommandStruct **)0x0;
  local_18 = 0xc;
  local_14 = 1;
  local_20 = param_1;
  local_1c = param_2;
  puVar1 = (undefined8 *)AppendDCLCommandList(this,(FWDCLCommandListElement *)&local_20,2,&local_28)
  ;
  if (puVar1 != (undefined8 *)0x0) {
    for (; puVar1 != (undefined8 *)0x0; puVar1 = (undefined8 *)*puVar1) {
      if (*(int *)(puVar1 + 2) == 0xc) {
        *(uint *)(puVar1 + 4) = param_2;
        puVar1[3] = local_28;
        return 1;
      }
    }
  }
  return 0;
}


