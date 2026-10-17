// FWSettingsLevels::InitGroupProp
// Entry: 00021444
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::InitGroupProp(char const*, m_audio_FWb::BaseChannelGroup*,
   FWSettingsLevels::ChannelGroupProperties&, bool, m_audio_FWb::BaseChannelGroup*) */

void __thiscall
FWSettingsLevels::InitGroupProp
          (FWSettingsLevels *this,char *param_1,BaseChannelGroup *param_2,
          ChannelGroupProperties *param_3,bool param_4,BaseChannelGroup *param_5)

{
  int iVar1;
  char local_70 [64];
  
  _snprintf(local_70,0x40,"MA%sLevels",param_1);
  iVar1 = (*(code *)PTR_init_00035078)(param_3,*(undefined8 *)(this + 0xa50),local_70,param_2,0,0);
  if (iVar1 == 0) {
    _snprintf(local_70,0x40,"MA%sMute",param_1);
    iVar1 = (*(code *)PTR_init_00035078)
                      (param_3 + 0x78,*(undefined8 *)(this + 0xa50),local_70,param_2,8,0);
    if (iVar1 == 0) {
      if (param_4) {
        _snprintf(local_70,0x40,"MA%sSolo",param_1);
        iVar1 = (*(code *)PTR_init_00035078)
                          (param_3 + 0x168,*(undefined8 *)(this + 0xa50),local_70,param_2,0x10,
                           param_5);
        if (iVar1 != 0) {
          return;
        }
      }
      _snprintf(local_70,0x40,"MA%sLink",param_1);
      (*(code *)PTR_init_00035078)
                (param_3 + 0xf0,*(undefined8 *)(this + 0xa50),local_70,param_2,0x18,0);
    }
  }
  return;
}
