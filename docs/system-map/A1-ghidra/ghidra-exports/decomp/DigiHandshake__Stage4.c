// DigiHandshake::Stage4
// Entry: 0001817e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* DigiHandshake::Stage4(int, int) */

int DigiHandshake::Stage4(int param_1,int param_2)

{
  s3 = param_2;
  return param_1 + 0x496143 + (param_1 / 0x2cf535) * -0x2cf535;
}
