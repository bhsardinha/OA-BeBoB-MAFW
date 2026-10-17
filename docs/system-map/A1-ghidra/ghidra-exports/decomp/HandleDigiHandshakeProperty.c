// HandleDigiHandshakeProperty
// Entry: 000180a8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* HandleDigiHandshakeProperty(char const*) */

undefined8 HandleDigiHandshakeProperty(char *param_1)

{
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  ulonglong uVar4;
  
  iVar1 = _strncmp(param_1,"LatencyProfiler",(long)sDigiHandshakePropertyNameStrLen);
  if (iVar1 == 0) {
    if (param_1[sDigiHandshakePropertyNameStrLen] == '\0') {
      uVar4 = 0;
    }
    else {
      lVar2 = _strtol(param_1 + sDigiHandshakePropertyNameStrLen,(char **)0x0,10);
      iVar1 = DigiHandshake::Do((int)lVar2);
      uVar4 = (ulonglong)iVar1;
    }
    uVar3 = OSNumber::withNumber(uVar4,0x20);
    return uVar3;
  }
  return 0;
}
