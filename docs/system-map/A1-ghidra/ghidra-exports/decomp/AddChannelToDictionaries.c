// AddChannelToDictionaries
// Entry: 00002bf4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e10)

/* AddChannelToDictionaries(OSDictionary*, OSDictionary*, OSDictionary*, unsigned int, unsigned int,
   char const*, char const*) */

void AddChannelToDictionaries
               (OSDictionary *param_1,OSDictionary *param_2,OSDictionary *param_3,uint param_4,
               uint param_5,char *param_6,char *param_7)

{
  long *plVar1;
  long *plVar2;
  long *plVar3;
  undefined4 in_register_0000000c;
  char local_f0 [64];
  char local_b0 [32];
  char local_90 [64];
  char local_50 [32];
  
  _sprintf(local_50,"%sChannel%u",param_6,CONCAT44(in_register_0000000c,param_4));
  _sprintf(local_90,"%s %s",param_6,param_7);
  _sprintf(local_b0,"%d",(ulong)param_5);
  _sprintf(local_f0,"%s %s",local_90,local_b0);
  plVar1 = (long *)OSString::withCString(local_f0);
  plVar2 = (long *)OSString::withCString(local_90);
  plVar3 = (long *)OSString::withCString(local_b0);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*(long *)param_1 + 0x1d0))(param_1,local_50,plVar1);
    (**(code **)(*plVar1 + 0x28))(plVar1);
  }
  if (plVar2 != (long *)0x0) {
    (**(code **)(*(long *)param_2 + 0x1d0))(param_2,local_50,plVar2);
    (**(code **)(*plVar2 + 0x28))(plVar2);
  }
  if (plVar3 != (long *)0x0) {
    (**(code **)(*(long *)param_3 + 0x1d0))(param_3,local_50,plVar3);
    (**(code **)(*plVar3 + 0x28))(plVar3);
  }
  return;
}
