// m_audio_b_FWAudioDevice::SetChannelNamesForMainEngine
// Entry: 00002d2a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e6)

/* m_audio_b_FWAudioDevice::SetChannelNamesForMainEngine(IOAudioEngine*) */

void __thiscall
m_audio_b_FWAudioDevice::SetChannelNamesForMainEngine
          (m_audio_b_FWAudioDevice *this,IOAudioEngine *param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  bool bVar4;
  long lVar5;
  OSDictionary *pOVar6;
  OSDictionary *pOVar7;
  OSDictionary *pOVar8;
  char *pcVar9;
  long lVar10;
  char *pcVar11;
  char *pcVar12;
  uint uVar13;
  uint uVar14;
  uint uVar15;
  uint *puVar16;
  ulong local_70;
  uint local_48 [6];
  
  lVar5 = (**(code **)(*(long *)this + 0xa60))(this);
  uVar14 = *(int *)(lVar5 + 0xc) + *(int *)(lVar5 + 4);
  local_48[4] = 0;
  local_48[5] = 0;
  local_48[2] = 0;
  local_48[3] = 0;
  local_48[0] = 0;
  local_48[1] = 0;
  if ((*(long *)(this + 0xe8) == 0) || (*(uint *)(*(long *)(this + 0xe8) + 0x8c) < 0x15888)) {
    bVar4 = false;
  }
  else {
    bVar4 = true;
  }
  pOVar6 = (OSDictionary *)OSDictionary::withCapacity(uVar14);
  pOVar7 = (OSDictionary *)OSDictionary::withCapacity(uVar14);
  pOVar8 = (OSDictionary *)OSDictionary::withCapacity(uVar14);
  if (((pOVar6 == (OSDictionary *)0x0) || (pOVar7 == (OSDictionary *)0x0)) ||
     (pOVar8 == (OSDictionary *)0x0)) {
    if (pOVar6 != (OSDictionary *)0x0) {
      (**(code **)(*(long *)pOVar6 + 0x28))(pOVar6);
    }
    if (pOVar7 != (OSDictionary *)0x0) {
      (**(code **)(*(long *)pOVar7 + 0x28))(pOVar7);
    }
    if (pOVar8 != (OSDictionary *)0x0) {
      (**(code **)(*(long *)pOVar8 + 0x28))(pOVar8);
    }
  }
  else {
    if (*(int *)(lVar5 + 0x4c) != 3) {
      pcVar9 = "ADAT SMUX";
      if (!bVar4) {
        pcVar9 = "ADAT";
      }
      local_70 = 0;
      do {
        lVar10 = local_70 * 0x10;
        cVar1 = *(char *)(lVar5 + 0x40 + lVar10);
        pcVar11 = "Input";
        if (cVar1 == '\0') {
          pcVar11 = "Output";
        }
        iVar2 = *(int *)(lVar5 + 0x4c + lVar10);
        local_70 = (ulong)((int)local_70 + 1);
        uVar14 = *(uint *)(lVar5 + 0x48 + lVar10);
        uVar15 = *(uint *)(lVar5 + 0x44 + lVar10);
        if (iVar2 == 1) {
          puVar16 = local_48 + (ulong)(cVar1 == '\0') + 4;
          pcVar12 = "S/PDIF";
        }
        else if (iVar2 == 2) {
          puVar16 = local_48 + (ulong)(cVar1 == '\0') + 2;
          pcVar12 = pcVar9;
        }
        else {
          puVar16 = local_48 + (cVar1 == '\0');
          pcVar12 = "Analog";
        }
        if (uVar15 <= uVar14) {
          uVar13 = uVar15 * 2;
          do {
            uVar3 = *puVar16;
            *puVar16 = uVar3 + 1;
            AddChannelToDictionaries(pOVar6,pOVar7,pOVar8,uVar13 - 1,uVar3 + 1,pcVar11,pcVar12);
            uVar3 = *puVar16;
            *puVar16 = uVar3 + 1;
            AddChannelToDictionaries(pOVar6,pOVar7,pOVar8,uVar13,uVar3 + 1,pcVar11,pcVar12);
            uVar13 = uVar13 + 2;
            uVar15 = uVar15 + 1;
          } while (uVar15 <= uVar14);
        }
      } while (*(int *)(lVar5 + 0x4c + local_70 * 0x10) != 3);
    }
    (**(code **)(*(long *)param_1 + 0x268))(param_1,"IOAudioEngineChannelNames",pOVar6);
    (**(code **)(*(long *)param_1 + 0x268))(param_1,"IOAudioEngineChannelCategoryNames",pOVar7);
    (**(code **)(*(long *)param_1 + 0x268))(param_1,"IOAudioEngineChannelNumberNames",pOVar8);
    (**(code **)(*(long *)pOVar6 + 0x28))(pOVar6);
    (**(code **)(*(long *)pOVar7 + 0x28))(pOVar7);
    (**(code **)(*(long *)pOVar8 + 0x28))(pOVar8);
  }
  return;
}
