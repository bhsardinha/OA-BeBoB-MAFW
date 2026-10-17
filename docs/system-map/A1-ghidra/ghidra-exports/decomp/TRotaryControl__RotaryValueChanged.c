// TRotaryControl::RotaryValueChanged
// Entry: 0000c2ba
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* TRotaryControl::RotaryValueChanged(unsigned int, int&) */

bool __thiscall TRotaryControl::RotaryValueChanged(TRotaryControl *this,uint param_1,int *param_2)

{
  int iVar1;
  
  *param_2 = 0;
  if (param_1 == 1) {
    iVar1 = *(int *)this;
  }
  else {
    if (param_1 != 2) {
      iVar1 = 0;
      goto LAB_0000c2dc;
    }
    iVar1 = -*(int *)this;
  }
  *param_2 = iVar1;
LAB_0000c2dc:
  return iVar1 != 0;
}
