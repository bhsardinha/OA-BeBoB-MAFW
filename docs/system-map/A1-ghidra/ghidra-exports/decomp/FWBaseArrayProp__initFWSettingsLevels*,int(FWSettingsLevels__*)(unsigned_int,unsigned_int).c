// m_audio_b_FWBaseArrayProp::init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
// Entry: 000226ca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* int m_audio_b_FWBaseArrayProp::init<FWSettingsLevels*, int (FWSettingsLevels::*)(unsigned int,
   unsigned int)>(m_audio_b_FWPropertyDict*, char const*, unsigned int, FWSettingsLevels*, int
   (FWSettingsLevels::*)(unsigned int, unsigned int)) */

int m_audio_b_FWBaseArrayProp::
    init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
              (m_audio_b_FWPropertyDict *param_1,char *param_2,uint param_3,
              FWSettingsLevels *param_4,_func_int_uint_uint *param_5)

{
  bool bVar1;
  int iVar2;
  undefined4 in_register_00000014;
  
  bVar1 = m_audio_FWb::
          TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
          ::Init<FWSettingsLevels*,int(FWSettingsLevels::*)(unsigned_int,unsigned_int)>
                    ((FWSettingsLevels *)(param_1 + 0x30),param_5);
  if (!bVar1) {
    return -0x1ffffd44;
  }
                    /* WARNING: Could not recover jumptable at 0x00022724. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar2 = (**(code **)(*(long *)param_1 + 0x148))
                    (param_1,param_2,CONCAT44(in_register_00000014,param_3),
                     (ulong)param_4 & 0xffffffff);
  return iVar2;
}
