// _RearrangeSampleFrame16Bit2
// Entry: 0002c615
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _RearrangeSampleFrame16Bit2(long param_1,long param_2,int param_3)

{
  ushort uVar1;
  long lVar2;
  undefined4 *puVar3;
  ushort *puVar4;
  
  puVar3 = (undefined4 *)(param_2 + 0xc);
  puVar4 = (ushort *)(param_1 + 2);
  lVar2 = (ulong)(param_3 - 1) + 1;
  do {
    uVar1 = *puVar4;
    puVar3[-3] = (uint)(ushort)(puVar4[-1] << 8 | puVar4[-1] >> 8) * 0x100 + 0x40000000;
    puVar3[-2] = 0x40000000;
    puVar3[-1] = (uint)(ushort)(uVar1 << 8 | uVar1 >> 8) * 0x100 + 0x40000000;
    *puVar3 = 0x40000000;
    puVar3 = puVar3 + 4;
    puVar4 = puVar4 + 2;
    lVar2 = lVar2 + -1;
  } while (lVar2 != 0);
  return;
}
