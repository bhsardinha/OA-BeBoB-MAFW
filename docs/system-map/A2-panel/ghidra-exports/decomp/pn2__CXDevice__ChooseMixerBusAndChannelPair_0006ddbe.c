// CXDevice::ChooseMixerBusAndChannelPair
// Entry: 0006ddbe
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ChooseMixerBusAndChannelPair(NSString*, NSString*&, unsigned short&) */

void __thiscall
CXDevice::ChooseMixerBusAndChannelPair
          (CXDevice *this,NSString *param_1,NSString **param_2,ushort *param_3)

{
  ushort uVar1;
  ushort uVar2;
  uint uVar3;
  cfstringStruct *pcVar4;
  NSString *pNVar5;
  
  uVar2 = (**(code **)(*(int *)this + 0x5c))(this);
  uVar2 = uVar2 >> 1;
  uVar1 = *param_3;
  if ((uVar1 == 0) || (uVar2 < uVar1)) {
    if (uVar2 < uVar1) {
      uVar3 = (**(code **)(*(int *)this + 0x1c))(this);
      if (uVar1 <= (ushort)((uVar3 & 0xfffe) >> 1)) {
        *param_3 = *param_3 - uVar2;
        pcVar4 = &cf_MASWReturn;
        goto LAB_0006de30;
      }
    }
    *param_3 = 0;
    pcVar4 = &cf_MAInvalid;
  }
  else {
    pcVar4 = &cf_MAInput;
  }
LAB_0006de30:
  pNVar5 = (NSString *)_objc_msgSend(pcVar4,PTR_s_stringByAppendingString__001d4098,param_1);
  *param_2 = pNVar5;
  return;
}
