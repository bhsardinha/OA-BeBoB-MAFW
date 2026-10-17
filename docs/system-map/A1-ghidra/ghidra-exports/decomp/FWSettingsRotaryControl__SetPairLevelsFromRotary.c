// FWSettingsRotaryControl::SetPairLevelsFromRotary
// Entry: 00021204
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::SetPairLevelsFromRotary(char const*, unsigned int, OSArray*&, int) */

void __thiscall
FWSettingsRotaryControl::SetPairLevelsFromRotary
          (FWSettingsRotaryControl *this,char *param_1,uint param_2,OSArray **param_3,int param_4)

{
  OSMetaClassBase *pOVar1;
  long lVar2;
  OSArray *pOVar3;
  bool bVar4;
  OSArray **ppOVar5;
  bool bVar6;
  int iVar7;
  
  pOVar3 = *param_3;
  ppOVar5 = param_3;
  iVar7 = param_4;
  if (pOVar3 != (OSArray *)0x0) {
LAB_00021257:
    bVar4 = SUB81(ppOVar5,0);
    bVar6 = SUB41(iVar7,0);
    AdjustOSNumberElement(pOVar3,param_2 * 2,param_4,bVar4,bVar6);
    AdjustOSNumberElement(*param_3,param_2 * 2 + 1,param_4,bVar4,bVar6);
    return;
  }
  pOVar1 = (OSMetaClassBase *)
           m_audio_b_FWPropertyDict::CopyProperty
                     (*(m_audio_b_FWPropertyDict **)(this + 0xc0),param_1);
  *param_3 = (OSArray *)pOVar1;
  if (pOVar1 != (OSMetaClassBase *)0x0) {
    ppOVar5 = (OSArray **)PTR_metaClass_00030078;
    lVar2 = OSMetaClassBase::safeMetaCast(pOVar1,*(OSMetaClass **)PTR_metaClass_00030078);
    if (lVar2 != 0) {
      pOVar3 = *param_3;
      goto LAB_00021257;
    }
  }
  return;
}
