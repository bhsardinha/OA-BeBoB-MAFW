// CFW1814DeviceSettings::WriteSettingsToHardware
// Entry: 0005bf10
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::WriteSettingsToHardware() */

undefined4 __thiscall CFW1814DeviceSettings::WriteSettingsToHardware(CFW1814DeviceSettings *this)

{
  ushort uVar1;
  uint uVar2;
  short sVar3;
  uint uVar4;
  ushort uVar5;
  CFW1814DeviceSettings *pCVar6;
  uint uVar7;
  int iVar8;
  bool bVar9;
  CFW1814DeviceSettings *local_18;
  CFW1814DeviceSettings local_11;
  
  local_11 = (CFW1814DeviceSettings)0x0;
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x5e5,2,this + 0x88);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x5e6,1,this + 0x8a);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x5e7,1,this + 0x8b);
  pCVar6 = this + 0x10c;
  uVar5 = 1;
  do {
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5ea,uVar5,4,pCVar6 + -0x80);
    if (uVar5 < 0x13) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5eb,uVar5,4,pCVar6);
    }
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5ec,uVar5,4,pCVar6 + 0x80);
    uVar5 = uVar5 + 1;
    pCVar6 = pCVar6 + 4;
  } while (uVar5 != 0x17);
  local_18 = this + 0x24c;
  uVar4 = 0;
  do {
    uVar2 = uVar4 + 1;
    uVar7 = uVar2 & 0xffff;
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5ed,uVar7,1,this + uVar4 + 0x20c);
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5ee,uVar7,1,this + uVar4 + 0x21c);
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5ef,uVar7,1,this + uVar4 + 0x22c);
    (**(code **)(**(int **)(this + 0x68) + 0x2c))
              (*(int **)(this + 0x68),0x5f0,uVar7,1,this + uVar4 + 0x23c);
    (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5f1,uVar7,2,local_18);
    local_18 = local_18 + 2;
    uVar4 = uVar2;
  } while ((short)uVar2 != 0xb);
  pCVar6 = this + 0x26c;
  bVar9 = true;
  uVar5 = 2;
  do {
    uVar1 = uVar5 - 1;
    if ((uVar1 < 5) || ((0xe < uVar1 && (!(bool)(bVar9 ^ 1))))) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5f4,uVar1,4,pCVar6);
    }
    uVar1 = uVar5 + 1;
    pCVar6 = pCVar6 + 4;
    bVar9 = uVar5 < 0x15;
    uVar5 = uVar1;
  } while (uVar1 != 0x16);
  bVar9 = true;
  iVar8 = 0x35c;
  uVar4 = 0;
  do {
    uVar2 = uVar4 + 1;
    uVar5 = (ushort)uVar2;
    if ((uVar5 < 3) || ((7 < uVar5 && (!(bool)(bVar9 ^ 1))))) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5f8,uVar2 & 0xffff,1,this + uVar4 + 0x33e);
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5fa,uVar2 & 0xffff,1,this + uVar4 + 0x352);
    }
    if ((uVar5 < 0xb) && ((0x407U >> (uVar2 & 0x1f) & 1) != 0)) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5f7,uVar2 & 0xffff,1,this + uVar4 + 0x334);
    }
    if (uVar5 < 3) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5f9,uVar2 & 0xffff,1,this + uVar4 + 0x348);
    }
    if (uVar5 < 4) {
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5fb,uVar2 & 0xffff,2,this + iVar8);
    }
    iVar8 = iVar8 + 2;
    bVar9 = (ushort)((short)uVar4 + 2U) < 0xb;
    uVar4 = uVar2;
  } while (uVar5 != 10);
  (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5fd,8,2,this + 0x370);
  (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5fd,9,2,this + 0x372);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x5ff,2,this + 900);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x601,4,this + 0x378);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x60d,2,this + 0x38c);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x60e,2,this + 0x38e);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x611,1,this + 0x39b);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x60f,2,this + 0x390);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x608,2,this + 0x392);
  (**(code **)(**(int **)(this + 0x68) + 0x28))(*(int **)(this + 0x68),0x609,6,this + 0x394);
  sVar3 = (**(code **)(**(int **)(this + 0x68) + 0x20))(*(int **)(this + 0x68),0x5fe,1,&local_11);
  if (sVar3 != 0) {
    local_11 = (CFW1814DeviceSettings)0x1;
  }
  this[0x374] = local_11;
  return 0;
}
