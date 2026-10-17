// _MixChannelPairToMultichannelBuffer
// Entry: 00028e57
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

void _MixChannelPairToMultichannelBuffer
               (uint param_1,uint param_2,uint param_3,long param_4,long param_5)

{
  float *pfVar1;
  long lVar2;
  float fVar3;
  float fVar4;
  float *pfVar5;
  int iVar6;
  long lVar7;
  
  if ((param_1 & 0x7fffffff) != 0) {
    pfVar5 = (float *)(param_4 + 4);
    lVar2 = param_5 + (ulong)(param_3 + 1) * 4;
    param_5 = param_5 + (ulong)param_3 * 4;
    iVar6 = param_1 * -2;
    lVar7 = 0;
    do {
      fVar3 = *(float *)(lVar2 + lVar7 * 4);
      pfVar1 = pfVar5 + -1;
      fVar4 = *pfVar5;
      pfVar5 = pfVar5 + 2;
      *(float *)(param_5 + lVar7 * 4) = *(float *)(param_5 + lVar7 * 4) + *pfVar1;
      *(float *)(lVar2 + lVar7 * 4) = fVar3 + fVar4;
      lVar7 = lVar7 + (ulong)param_2;
      iVar6 = iVar6 + 2;
    } while (iVar6 != 0);
  }
  return;
}
