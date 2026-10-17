// CXDevice::ChooseOutputBusAndChannelPair
// Entry: 0006dc08
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ChooseOutputBusAndChannelPair(NSString*, NSString*&, unsigned short&) */

void __thiscall
CXDevice::ChooseOutputBusAndChannelPair
          (CXDevice *this,NSString *param_1,NSString **param_2,ushort *param_3)

{
  ushort uVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  NSString *pNVar5;
  cfstringStruct *pcVar6;
  uint uVar7;
  ushort uVar8;
  uint uVar9;
  
  uVar3 = (**(code **)(*(int *)this + 0x18))(this);
  uVar1 = (**(code **)(*(int *)this + 0x60))(this);
  uVar2 = (**(code **)(*(int *)this + 0x18))(this);
  iVar4 = (**(code **)(*(int *)this + 0x60))(this);
  uVar9 = ((uVar2 - 2) - iVar4 & 0xfffe) >> 1;
  uVar2 = *param_3;
  uVar7 = (uint)uVar2;
  uVar8 = (ushort)uVar9;
  if ((uVar7 == 0) || (uVar8 < uVar2)) {
    uVar9 = uVar9 + (uVar1 >> 1);
    if ((uVar9 < uVar7) || (uVar2 <= uVar8)) {
      if ((uVar7 - uVar9 == 0 || uVar7 < uVar9) || ((ushort)((uVar3 & 0xfffe) >> 1) < uVar2)) {
        *param_3 = 0;
        pcVar6 = &cf_MAInvalid;
      }
      else {
        *param_3 = (ushort)(uVar7 - uVar9);
        pcVar6 = &cf_MAAuxOutput;
      }
    }
    else {
      *param_3 = uVar2 - uVar8;
      pcVar6 = &cf_MAHeadphone;
    }
  }
  else {
    pcVar6 = &cf_MAOutput;
  }
  pNVar5 = (NSString *)_objc_msgSend(pcVar6,PTR_s_stringByAppendingString__001d4098,param_1);
  *param_2 = pNVar5;
  return;
}
