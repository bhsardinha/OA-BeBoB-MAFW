// FWSettings410Levels::SendToDevice
// Entry: 00014046
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings410Levels::SendToDevice() */

int __thiscall FWSettings410Levels::SendToDevice(FWSettings410Levels *this)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_SendToDevice_00048e60)(this);
  if (iVar1 == 0) {
    (**(code **)(*(long *)this + 0x78))();
  }
  return iVar1;
}
