// com_m_audio_FW1814Device::WorkaroundFor1814InputDelay
// Entry: 0000cba4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

/* com_m_audio_FW1814Device::WorkaroundFor1814InputDelay(unsigned int, int*, int*, void*) */

void com_m_audio_FW1814Device::WorkaroundFor1814InputDelay
               (uint param_1,int *param_2,int *param_3,void *param_4)

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
  uint uVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  ulong uVar15;
  int *piVar16;
  uint *puVar17;
  int local_64;
  
  if (param_1 != 0) {
    uVar1 = *(uint *)(*(long *)((long)param_4 + 0xe8) + 0x2e4);
    local_64 = -param_1;
    puVar17 = (uint *)(param_2 + 0xf);
    piVar16 = param_3 + 0xf;
    do {
      uVar14 = *(int *)((long)param_4 + 0x1d4c) + 1;
      *(uint *)((long)param_4 + 0x1d4c) = uVar14;
      uVar15 = (ulong)*(uint *)((long)param_4 + (ulong)(uVar14 & 0xf) * 4 + 0x1b8c);
      uVar14 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1be0);
      uVar2 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd4);
      uVar3 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1bdc);
      uVar4 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd0);
      uVar5 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd8);
      uVar6 = *(uint *)((long)param_4 + uVar15 * 4 + 0x1bcc);
      uVar7 = puVar17[-0xb];
      uVar8 = puVar17[-8];
      uVar9 = puVar17[-9];
      uVar10 = puVar17[-10];
      uVar11 = puVar17[-0xc];
      uVar12 = puVar17[-0xd];
      uVar13 = puVar17[-0xf];
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1bcc) = puVar17[-0xe];
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd0) = uVar12;
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd4) = uVar11;
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1bd8) = uVar10;
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1bdc) = uVar9;
      *(uint *)((long)param_4 + uVar15 * 4 + 0x1be0) = uVar8;
      piVar16[-0xf] =
           (int)(((uVar13 & 0xff00) << 8 | uVar13 >> 0x18) * 0x100 + (uVar13 & 0xff0000)) >> 8;
      piVar16[-0xe] =
           (int)(((uVar7 & 0xff00) << 8 | uVar7 >> 0x18) * 0x100 + (uVar7 & 0xff0000)) >> 8;
      piVar16[-0xd] =
           (int)(((uVar6 & 0xff00) << 8 | uVar6 >> 0x18) * 0x100 + (uVar6 & 0xff0000)) >> 8;
      piVar16[-0xc] =
           (int)(((uVar5 & 0xff00) << 8 | uVar5 >> 0x18) * 0x100 + (uVar5 & 0xff0000)) >> 8;
      piVar16[-0xb] =
           (int)(((uVar4 & 0xff00) << 8 | uVar4 >> 0x18) * 0x100 + (uVar4 & 0xff0000)) >> 8;
      piVar16[-10] = (int)(((uVar3 & 0xff00) << 8 | uVar3 >> 0x18) * 0x100 + (uVar3 & 0xff0000)) >>
                     8;
      piVar16[-9] = (int)(((uVar2 & 0xff00) << 8 | uVar2 >> 0x18) * 0x100 + (uVar2 & 0xff0000)) >> 8
      ;
      piVar16[-8] = (int)(((uVar14 & 0xff00) << 8 | uVar14 >> 0x18) * 0x100 + (uVar14 & 0xff0000))
                    >> 8;
      uVar14 = puVar17[-7];
      uVar2 = puVar17[-6];
      if (2 < uVar1 - 8) {
        uVar3 = puVar17[-5];
        uVar4 = puVar17[-4];
        if (4 < uVar1 - 8) {
          uVar5 = puVar17[-3];
          uVar6 = puVar17[-2];
          uVar7 = *puVar17;
          uVar8 = puVar17[-1];
          piVar16[-3] = (int)(((uVar5 & 0xff00) << 8 | uVar5 >> 0x18) * 0x100 + (uVar5 & 0xff0000))
                        >> 8;
          piVar16[-2] = (int)(((uVar6 & 0xff00) << 8 | uVar6 >> 0x18) * 0x100 + (uVar6 & 0xff0000))
                        >> 8;
          piVar16[-1] = (int)(((uVar8 & 0xff00) << 8 | uVar8 >> 0x18) * 0x100 + (uVar8 & 0xff0000))
                        >> 8;
          *piVar16 = (int)(((uVar7 & 0xff00) << 8 | uVar7 >> 0x18) * 0x100 + (uVar7 & 0xff0000)) >>
                     8;
        }
        piVar16[-5] = (int)(((uVar3 & 0xff00) << 8 | uVar3 >> 0x18) * 0x100 + (uVar3 & 0xff0000)) >>
                      8;
        piVar16[-4] = (int)(((uVar4 & 0xff00) << 8 | uVar4 >> 0x18) * 0x100 + (uVar4 & 0xff0000)) >>
                      8;
      }
      piVar16[-7] = (int)(((uVar14 & 0xff00) << 8 | uVar14 >> 0x18) * 0x100 + (uVar14 & 0xff0000))
                    >> 8;
      piVar16[-6] = (int)(((uVar2 & 0xff00) << 8 | uVar2 >> 0x18) * 0x100 + (uVar2 & 0xff0000)) >> 8
      ;
      puVar17 = puVar17 + (uVar1 + 1);
      piVar16 = piVar16 + uVar1;
      local_64 = local_64 + uVar1;
    } while (local_64 != 0);
  }
  return;
}
