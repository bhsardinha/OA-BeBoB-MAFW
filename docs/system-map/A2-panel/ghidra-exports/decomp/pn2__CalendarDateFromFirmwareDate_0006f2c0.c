// CalendarDateFromFirmwareDate
// Entry: 0006f2c0
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (panel build, About tab)

/* CalendarDateFromFirmwareDate(unsigned long, unsigned long, unsigned long, unsigned long) */

undefined4 __regparm3
CalendarDateFromFirmwareDate(ulong param_1,ulong param_2,ulong param_3,ulong param_4)

{
  undefined *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  ulong local_9c;
  undefined1 uStack_98;
  undefined3 uStack_97;
  undefined1 uStack_94;
  undefined1 uStack_93;
  ulong uStack_92;
  undefined1 uStack_8e;
  undefined1 uStack_89;
  undefined4 uStack_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 local_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 uStack_70;
  undefined4 local_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  undefined4 uStack_60;
  undefined4 local_5c;
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  undefined4 local_4c;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined4 uStack_40;
  undefined4 local_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 local_2c;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined4 uStack_20;
  int local_14;
  
  puVar1 = PTR____stack_chk_guard_00163028;
  local_14 = *(int *)PTR____stack_chk_guard_00163028;
  local_2c = 0;
  uStack_28 = 0;
  uStack_24 = 0;
  uStack_20 = 0;
  local_3c = 0;
  uStack_38 = 0;
  uStack_34 = 0;
  uStack_30 = 0;
  local_4c = 0;
  uStack_48 = 0;
  uStack_44 = 0;
  uStack_40 = 0;
  local_5c = 0;
  uStack_58 = 0;
  uStack_54 = 0;
  uStack_50 = 0;
  local_6c = 0;
  uStack_68 = 0;
  uStack_64 = 0;
  uStack_60 = 0;
  local_7c = 0;
  uStack_78 = 0;
  uStack_74 = 0;
  uStack_70 = 0;
  uStack_89 = 0;
  uStack_88 = 0;
  uStack_84 = 0;
  uStack_80 = 0;
  _uStack_98 = CONCAT31((int3)param_2,0x20);
  uStack_94 = (undefined1)(param_2 >> 0x18);
  uStack_93 = 0x20;
  uStack_8e = 0x20;
  local_9c = param_3;
  uStack_92 = param_4;
  uVar2 = _objc_msgSend(PTR_s_NSString_001d4794,PTR_s_stringWithCString__001d40d8,&local_9c);
  uVar2 = _objc_msgSend(PTR_s_NSCalendarDate_001d47a0,PTR_s_dateWithString_calendarFormat__001d40dc,
                        uVar2,&cf__Y_m_d_H_M_S);
  uVar3 = _objc_msgSend(PTR_s_NSCalendarDate_001d47a0,PTR_s_calendarDate_001d40e0);
  uVar3 = _objc_msgSend(uVar3,PTR_s_calendarFormat_001d40e4);
  _objc_msgSend(uVar2,PTR_s_setCalendarFormat__001d40e8,uVar3);
  if (*(int *)puVar1 == local_14) {
    return uVar2;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}
