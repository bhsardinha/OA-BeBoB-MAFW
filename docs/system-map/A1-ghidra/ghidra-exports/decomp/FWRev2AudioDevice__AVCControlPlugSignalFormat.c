// FWRev2AudioDevice::AVCControlPlugSignalFormat
// Entry: 00012e7e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* WARNING: Type propagation algorithm not settling */
/* FWRev2AudioDevice::AVCControlPlugSignalFormat(m_audio_b_FWAVCAudioDevice::PlugDirection, unsigned
   char, FDFValue, unsigned int, m_audio_b_FWAVCAudioDevice::PlugFormat) */

int FWRev2AudioDevice::AVCControlPlugSignalFormat
              (long *param_1,int param_2,undefined1 param_3,undefined4 param_4)

{
  byte bVar1;
  long *plVar2;
  code *pcVar3;
  long lVar4;
  uint uVar5;
  int iVar6;
  void *pvVar7;
  undefined8 *puVar8;
  ulong uVar9;
  char cVar10;
  ulong uVar11;
  undefined1 *puVar12;
  char cVar13;
  undefined8 uStack_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined1 local_51;
  long *local_50;
  uint local_48 [2];
  undefined1 local_3e;
  undefined1 local_3d;
  undefined1 local_3c;
  undefined1 local_3b;
  undefined1 local_3a;
  undefined1 local_39;
  undefined1 local_38;
  undefined1 local_37;
  undefined1 local_36;
  undefined1 local_35;
  char local_34;
  
  switch(param_4) {
  case 0:
    cVar13 = '\x02';
    break;
  default:
    cVar13 = '\x03';
    break;
  case 2:
    cVar13 = '\x04';
    break;
  case 3:
    cVar13 = '\n';
    break;
  case 4:
    cVar13 = '\x05';
    break;
  case 5:
    cVar13 = '\x06';
    break;
  case 6:
    cVar13 = '\a';
  }
  uStack_70 = 0x12edf;
  local_51 = param_3;
  local_50 = param_1;
  (**(code **)(*param_1 + 0xa60))();
  uStack_70 = 0x12ee9;
  pvVar7 = operator_new__(0x200);
  if (pvVar7 == (void *)0x0) {
    iVar6 = -0x1ffffd43;
  }
  else {
    cVar10 = '\0';
    do {
      local_48[1] = 0x200;
      local_3e = 1;
      local_3d = 0xff;
      local_3c = 0x2f;
      local_3b = 0xc1;
      local_39 = 0;
      local_38 = 0;
      local_37 = local_51;
      local_36 = 0xff;
      local_35 = 0xff;
      uStack_70 = 0x12f62;
      local_3a = param_2 == 0;
      local_34 = cVar10;
      iVar6 = (**(code **)(*(long *)local_50[0x4d] + 0x8b8))
                        ((long *)local_50[0x4d],&local_3e,0xe,pvVar7,local_48 + 1);
      if (*(char *)((long)pvVar7 + 0xd) == '\0') {
        iVar6 = -0x1ffffd44;
        puVar8 = &local_68;
        goto LAB_00013037;
      }
      puVar8 = &local_68;
      if (iVar6 != 0) goto LAB_00013037;
      cVar10 = cVar10 + '\x01';
    } while (*(char *)((long)pvVar7 + 0xd) != cVar13);
    local_48[0] = *(byte *)((long)pvVar7 + 0xf) + 0x12 + (uint)*(byte *)((long)pvVar7 + 0xf);
    uVar9 = (ulong)local_48[0];
    uVar11 = uVar9 + 0xf & 0xfffffffffffffff0;
    lVar4 = -uVar11;
    puVar12 = (undefined1 *)((long)&local_68 + lVar4);
    local_68 = &local_68;
    local_60 = &local_68;
    *(undefined8 *)((long)&uStack_70 + lVar4) = 0x12fab;
    _bzero(puVar12,uVar9);
    *puVar12 = 0;
    *(undefined1 *)((long)&local_68 + lVar4 + 1) = 0xff;
    *(undefined1 *)((long)&local_68 + lVar4 + 2) = 0x2f;
    *(undefined1 *)((long)&local_68 + lVar4 + 3) = 0xc0;
    *(bool *)((long)&local_68 + lVar4 + 4) = param_2 == 0;
    *(undefined1 *)((long)&local_68 + lVar4 + 5) = 0;
    *(undefined1 *)((long)&local_68 + lVar4 + 6) = 0;
    *(undefined1 *)((long)&local_68 + lVar4 + 7) = local_51;
    *(undefined1 *)((long)&local_60 + lVar4) = 0xff;
    *(undefined1 *)((long)&local_60 + lVar4 + 1) = 0xff;
    *(undefined1 *)((long)&local_60 + lVar4 + 2) = 0x90;
    *(undefined1 *)((long)&local_60 + lVar4 + 3) = 0x40;
    *(char *)((long)&local_60 + lVar4 + 4) = cVar13;
    *(undefined1 *)((long)local_68 + (0xd - uVar11)) = 1;
    bVar1 = *(byte *)((long)pvVar7 + 0xf);
    *(undefined8 *)((long)&uStack_70 + lVar4) = 0x13010;
    _memmove((void *)((long)&local_60 + lVar4 + 6),(void *)((long)pvVar7 + 0xf),(ulong)bVar1 * 2 + 1
            );
    uVar5 = local_48[0];
    plVar2 = (long *)local_50[0x4d];
    pcVar3 = *(code **)(*plVar2 + 0x8b8);
    *(undefined8 *)((long)&uStack_70 + lVar4) = 0x13031;
    iVar6 = (*pcVar3)(plVar2,puVar12,uVar5,pvVar7,local_48);
    puVar8 = local_60;
LAB_00013037:
    *(undefined8 *)((long)puVar8 + -8) = 0x1303f;
    operator_delete__(pvVar7);
  }
  return iVar6;
}
