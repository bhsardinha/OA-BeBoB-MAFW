// _ConvertInputFrames1212CS
// Entry: 00029183
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _ConvertInputFrames1212CS(uint param_1,long param_2,long param_3,uint param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  int *piVar11;
  uint *puVar12;
  uint local_38;
  
  if (param_1 != 0) {
    piVar11 = (int *)(param_3 + 0x3c);
    puVar12 = (uint *)(param_2 + 0x3c);
    local_38 = 0;
    do {
      uVar1 = puVar12[-0xf];
      uVar2 = puVar12[-0xe];
      uVar3 = puVar12[-0xd];
      uVar4 = puVar12[-0xc];
      uVar5 = puVar12[-0xb];
      uVar6 = puVar12[-10];
      uVar7 = puVar12[-9];
      uVar8 = puVar12[-8];
      uVar9 = puVar12[-7];
      uVar10 = puVar12[-6];
      piVar11[-0xf] =
           (int)((uVar1 & 0xff0000) + ((uVar1 & 0xff00) << 8 | uVar1 >> 0x18) * 0x100) >> 8;
      piVar11[-0xe] =
           (int)((uVar5 >> 0x18 | (uVar5 & 0xff0000) >> 8 | (uVar5 & 0xff00) << 8) << 8) >> 8;
      piVar11[-0xd] =
           (int)((uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8) << 8) >> 8;
      piVar11[-0xc] =
           (int)((uVar6 >> 0x18 | (uVar6 & 0xff0000) >> 8 | (uVar6 & 0xff00) << 8) << 8) >> 8;
      piVar11[-0xb] =
           (int)((uVar3 >> 0x18 | (uVar3 & 0xff0000) >> 8 | (uVar3 & 0xff00) << 8) << 8) >> 8;
      piVar11[-10] = (int)((uVar7 >> 0x18 | (uVar7 & 0xff0000) >> 8 | (uVar7 & 0xff00) << 8) << 8)
                     >> 8;
      piVar11[-9] = (int)((uVar4 >> 0x18 | (uVar4 & 0xff0000) >> 8 | (uVar4 & 0xff00) << 8) << 8) >>
                    8;
      piVar11[-8] = (int)((uVar8 >> 0x18 | (uVar8 & 0xff0000) >> 8 | (uVar8 & 0xff00) << 8) << 8) >>
                    8;
      piVar11[-7] = (int)((uVar9 >> 0x18 | (uVar9 & 0xff0000) >> 8 | (uVar9 & 0xff00) << 8) << 8) >>
                    8;
      piVar11[-6] = (int)((uVar10 >> 0x18 | (uVar10 & 0xff0000) >> 8 | (uVar10 & 0xff00) << 8) << 8)
                    >> 8;
      if (0xb < param_4) {
        uVar1 = puVar12[-5];
        uVar2 = puVar12[-4];
        piVar11[-5] = (int)((uVar1 >> 0x18 | (uVar1 & 0xff0000) >> 8 | (uVar1 & 0xff00) << 8) << 8)
                      >> 8;
        piVar11[-4] = (int)((uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8) << 8)
                      >> 8;
        if (0xd < param_4) {
          uVar1 = puVar12[-3];
          uVar2 = puVar12[-2];
          uVar3 = puVar12[-1];
          uVar4 = *puVar12;
          piVar11[-3] = (int)((uVar1 >> 0x18 | (uVar1 & 0xff0000) >> 8 | (uVar1 & 0xff00) << 8) << 8
                             ) >> 8;
          piVar11[-2] = (int)((uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8) << 8
                             ) >> 8;
          piVar11[-1] = (int)((uVar3 >> 0x18 | (uVar3 & 0xff0000) >> 8 | (uVar3 & 0xff00) << 8) << 8
                             ) >> 8;
          *piVar11 = (int)((uVar4 & 0xff0000) + ((uVar4 & 0xff00) << 8 | uVar4 >> 0x18) * 0x100) >>
                     8;
        }
      }
      piVar11 = piVar11 + param_4;
      puVar12 = puVar12 + (param_4 + 1);
      local_38 = local_38 + param_4;
    } while (local_38 < param_1);
  }
  return;
}
