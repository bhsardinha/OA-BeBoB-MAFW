// m_audio_b_FWAVCConnectionManager::updateP2PCount
// Entry: 000244f0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::updateP2PCount(unsigned int, int, bool, unsigned int,
   IOFWSpeed) */

int __thiscall
m_audio_b_FWAVCConnectionManager::updateP2PCount
          (m_audio_b_FWAVCConnectionManager *this,undefined4 param_1,int param_2,
          undefined4 param_4_00,uint param_4,uint param_6)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  long *plVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  long *plVar12;
  uint local_48;
  uint local_44;
  undefined2 local_40;
  undefined2 local_3e;
  undefined4 local_3c;
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_34;
  
  plVar12 = *(long **)(*(long *)(this + 0x10) + 0xe0);
  local_40 = 0;
  local_3e = 0xffff;
  local_3c = param_1;
  plVar8 = (long *)(**(code **)(*plVar12 + 0x888))(plVar12,&local_40,&local_44,1,0,0,param_4_00);
  iVar5 = (**(code **)(*plVar8 + 0x140))(plVar8,0);
  (**(code **)(*plVar8 + 0x28))(plVar8);
  if (iVar5 == 0) {
    uVar6 = (param_4 & 0x3f) << 0x10;
    uVar1 = local_44 >> 0x18 | (local_44 & 0xff0000) >> 8;
    uVar2 = (local_44 & 0xff00) << 8;
    uVar3 = local_44 << 0x18;
    local_44 = uVar1 | uVar2 | uVar3;
    uVar9 = local_44 | 0x80000000;
    iVar11 = -1;
    while (iVar11 = iVar11 + 1, iVar11 < 4) {
      uVar10 = uVar9;
      if (param_4 != 0xffffffff) {
        uVar10 = uVar1 | uVar2 & 0xffc0ffff | uVar3 | 0x80000000 | uVar6;
      }
      if (param_6 != 0x80000000) {
        uVar10 = uVar10 & 0xffff3fff | (param_6 & 3) << 0xe;
      }
      uVar7 = (uVar10 & 0xc0ffffff) + 0x1000000;
      if (param_2 < 1) {
        uVar7 = uVar10 & 0xc0ffffff;
      }
      local_48 = uVar7 >> 0x18 | (uVar7 & 0xff0000) >> 8 | (uVar7 & 0xff00) << 8 | uVar7 << 0x18;
      local_38 = 0;
      local_36 = 0xffff;
      local_34 = param_1;
      plVar8 = (long *)(**(code **)(*plVar12 + 0x8a0))
                                 (plVar12,&local_38,&local_44,&local_48,1,0,0,0,plVar12,uVar6,
                                  param_1);
      iVar5 = (**(code **)(*plVar8 + 0x140))(plVar8,0);
      cVar4 = (**(code **)(*plVar8 + 0x1f0))(plVar8,&local_44);
      (**(code **)(*plVar8 + 0x28))(plVar8);
      if (iVar5 != -0x1fff7fec) {
        if (iVar5 != 0) {
          return iVar5;
        }
        if (cVar4 != '\0') {
          return 0;
        }
      }
      _IOSleep(100);
    }
  }
  return iVar5;
}
