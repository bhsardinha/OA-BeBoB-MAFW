// CXDevice::SetChannelLevelProperty
// Entry: 0006d98e
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetChannelLevelProperty(NSString*, unsigned long, void*) */

void __thiscall
CXDevice::SetChannelLevelProperty(CXDevice *this,NSString *param_1,ulong param_2,void *param_3)

{
  SetChannelLevelProperty(this,param_1,param_3,1,param_2);
  return;
}
