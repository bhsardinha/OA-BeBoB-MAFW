// DigiHandshake::Do
// Entry: 000181ac
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* DigiHandshake::Do(int) */

uint DigiHandshake::Do(int param_1)

{
  uint uVar1;
  
  if (param_1 != 0) {
    switch(sStage) {
    case 0:
      break;
    case 1:
      sStage = 2;
      s2 = param_1 / 0x1f31d;
      uVar1 = (param_1 % 0x1f31d) * 0x41a7;
      s1 = uVar1 ^ 0x41a7;
      return uVar1;
    case 2:
      sStage = 3;
      s3 = param_1 + 0x496143 + (param_1 / 0x2cf535) * -0x2cf535;
      s2 = s2 * 0xb14;
      return (int)s2 % 0xb14;
    case 3:
      sStage = 4;
      uVar1 = param_1 - s2;
      if ((int)(param_1 - s2) < 0) {
        uVar1 = (param_1 - s2) + 0x7fffffff;
      }
      s2 = s2 ^ uVar1;
      s3 = (int)uVar1 % 0x1f31d;
      return uVar1;
    case 4:
      sStage = 0;
      s2 = (int)s1 % 0x41a7;
      s3 = ((int)s1 % 0x41a7) % 0x1f31d;
      return param_1 + 0x496143 + (param_1 / 0x2cf535) * -0x2cf535;
    default:
      return 0;
    }
  }
  s3 = 0;
  sStage = 1;
  s2 = 0;
  s1 = 0;
  return 0;
}
