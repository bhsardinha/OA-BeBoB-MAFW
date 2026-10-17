// __stop
// Entry: 0002d9b3
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

undefined8 __stop(void)

{
  undefined8 uVar1;
  
  if (__antimain == (code *)0x0) {
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0002d9cb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (*__antimain)();
  return uVar1;
}
