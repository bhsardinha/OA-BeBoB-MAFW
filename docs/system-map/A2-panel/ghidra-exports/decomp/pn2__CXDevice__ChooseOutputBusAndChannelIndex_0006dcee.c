// CXDevice::ChooseOutputBusAndChannelIndex
// Entry: 0006dcee
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ChooseOutputBusAndChannelIndex(NSString*, NSString*&, unsigned short&) */

void __thiscall
CXDevice::ChooseOutputBusAndChannelIndex
          (CXDevice *this,NSString *param_1,NSString **param_2,ushort *param_3)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  short sVar5;
  NSString *pNVar6;
  cfstringStruct *pcVar7;
  uint uVar8;
  uint uVar9;
  ushort uVar10;
  
  uVar2 = (**(code **)(*(int *)this + 0x18))(this);
  uVar3 = (**(code **)(*(int *)this + 0x60))(this);
  sVar4 = (**(code **)(*(int *)this + 0x18))(this);
  sVar5 = (**(code **)(*(int *)this + 0x60))(this);
  uVar10 = (sVar4 + -2) - sVar5;
  uVar1 = *param_3;
  uVar8 = (uint)uVar1;
  if ((uVar8 == 0) || (uVar10 < uVar1)) {
    uVar9 = (uint)uVar10 + (uint)uVar3;
    if ((uVar9 < uVar8) || (uVar1 <= uVar10)) {
      if ((uVar8 - uVar9 == 0 || uVar8 < uVar9) || (uVar2 < uVar1)) {
        *param_3 = 0;
        pcVar7 = &cf_MAInvalid;
      }
      else {
        *param_3 = (ushort)(uVar8 - uVar9);
        pcVar7 = &cf_MAAuxOutput;
      }
    }
    else {
      *param_3 = uVar1 - uVar10;
      pcVar7 = &cf_MAHeadphone;
    }
  }
  else {
    pcVar7 = &cf_MAOutput;
  }
  pNVar6 = (NSString *)_objc_msgSend(pcVar7,PTR_s_stringByAppendingString__001d4098,param_1);
  *param_2 = pNVar6;
  return;
}
