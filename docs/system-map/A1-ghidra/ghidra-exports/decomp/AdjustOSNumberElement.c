// AdjustOSNumberElement
// Entry: 00021148
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* AdjustOSNumberElement(OSArray*, unsigned int, int, bool, bool) */

void AdjustOSNumberElement(OSArray *param_1,uint param_2,int param_3,bool param_4,bool param_5)

{
  int iVar1;
  int iVar2;
  long *plVar3;
  int iVar4;
  undefined7 in_register_00000009;
  undefined4 in_register_00000014;
  
  plVar3 = (long *)(**(code **)(*(long *)param_1 + 0x1e8))
                             (param_1,param_2,CONCAT44(in_register_00000014,param_3),
                              CONCAT71(in_register_00000009,param_4));
  if (plVar3 != (long *)0x0) {
    iVar1 = (**(code **)(*plVar3 + 0x148))(plVar3);
    if ((iVar1 < -0xc) && (iVar1 + param_3 < -0xc)) {
      iVar4 = param_3 * 0xc;
    }
    else {
      iVar4 = -1;
      if (0 < param_3) {
        iVar4 = 1;
      }
    }
    iVar2 = -0x80;
    if (-0x81 < iVar4 + iVar1) {
      iVar2 = iVar4 + iVar1;
    }
    if (iVar2 < -0x100) {
      iVar2 = -0x100;
    }
    else if (0 < iVar2) {
      iVar2 = 0;
    }
    plVar3 = (long *)OSNumber::withNumber((long)iVar2,0x20);
    if (plVar3 != (long *)0x0) {
      (**(code **)(*(long *)param_1 + 0x1d0))(param_1,param_2,plVar3);
                    /* WARNING: Could not recover jumptable at 0x00021201. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*plVar3 + 0x28))(plVar3);
      return;
    }
  }
  return;
}
