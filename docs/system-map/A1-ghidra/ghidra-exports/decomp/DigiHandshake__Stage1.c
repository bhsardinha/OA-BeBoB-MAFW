// DigiHandshake::Stage1
// Entry: 0001812c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* DigiHandshake::Stage1(int, int*, int*) */

void DigiHandshake::Stage1(int param_1,int *param_2,int *param_3)

{
  *param_2 = (param_1 % 0x1f31d) * 0x41a7;
  *param_3 = param_1 / 0x1f31d;
  return;
}
