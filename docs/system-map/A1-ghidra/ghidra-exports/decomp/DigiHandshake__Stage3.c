// DigiHandshake::Stage3
// Entry: 00018168
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* DigiHandshake::Stage3(int, int, int*) */

void DigiHandshake::Stage3(int param_1,int param_2,int *param_3)

{
  int iVar1;
  
  iVar1 = param_2 - param_1;
  *param_3 = iVar1;
  if (iVar1 < 0) {
    *param_3 = iVar1 + 0x7fffffff;
  }
  return;
}
