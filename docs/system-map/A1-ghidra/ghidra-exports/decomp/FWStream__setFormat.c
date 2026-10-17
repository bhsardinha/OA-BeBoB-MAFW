// m_audio_b_FWStream::setFormat
// Entry: 00010bda
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWStream::setFormat(_IOAudioStreamFormat const*, _IOAudioStreamFormatExtension const*,
   OSDictionary*, bool) */

int __thiscall
m_audio_b_FWStream::setFormat
          (m_audio_b_FWStream *this,_IOAudioStreamFormat *param_1,
          _IOAudioStreamFormatExtension *param_2,OSDictionary *param_3,bool param_4)

{
  char cVar1;
  OSMetaClassBase *pOVar2;
  OSDictionary *pOVar3;
  int *piVar4;
  OSCollection *pOVar5;
  long *plVar6;
  long *plVar7;
  long lVar8;
  long *plVar9;
  uint uVar10;
  int iVar11;
  undefined8 *puVar12;
  int *local_b0;
  int local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  int local_94;
  undefined1 local_90 [40];
  long local_68;
  int local_60;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  
  plVar9 = *(long **)(this + 0x88);
  iVar11 = *(int *)(this + 0x150);
  *(int *)(this + 0x150) = iVar11 + 1;
  if ((param_1 == (_IOAudioStreamFormat *)0x0) || (param_3 == (OSDictionary *)0x0)) {
    return -0x1ffffd3e;
  }
  local_38 = *(undefined8 *)(param_1 + 0x10);
  local_48 = *(undefined8 *)param_1;
  local_40 = *(undefined8 *)(param_1 + 8);
  if (param_2 == (_IOAudioStreamFormatExtension *)0x0) {
    local_a0 = 1;
    local_9c = 0;
    local_98 = 1;
    local_94 = (uint)((byte)param_1[0xd] >> 3) * *(int *)param_1;
  }
  else {
    local_a0 = *(undefined4 *)param_2;
    local_9c = *(undefined4 *)(param_2 + 4);
    local_98 = *(undefined4 *)(param_2 + 8);
    local_94 = *(int *)(param_2 + 0xc);
  }
  cVar1 = (**(code **)(*(long *)this + 0x878))(this,&local_48,&local_a0,local_90);
  if (cVar1 == '\0') {
    _IOLog("IOAudioStream<0x%p>::setFormat(0x%p, 0x%p) - invalid format.\n",this,param_1,param_3);
    return -0x1ffffd3e;
  }
  pOVar2 = (OSMetaClassBase *)(**(code **)(*(long *)param_3 + 0x208))(param_3,"IOAudioSampleRate");
  pOVar3 = (OSDictionary *)
           OSMetaClassBase::safeMetaCast(pOVar2,*(OSMetaClass **)PTR_metaClass_00030008);
  if (pOVar3 == (OSDictionary *)0x0) {
LAB_00010d2c:
    local_b0 = (int *)0x0;
  }
  else {
    local_b0 = (int *)IOAudioEngine::createSampleRateFromDictionary
                                (pOVar3,(_IOAudioSampleRate *)0x0);
    piVar4 = (int *)(**(code **)(*plVar9 + 0xa78))(plVar9);
    if (((local_b0 != (int *)0x0) && (*local_b0 == *piVar4)) && (local_b0[1] == piVar4[1]))
    goto LAB_00010d2c;
  }
  pOVar5 = (OSCollection *)OSSet::withCapacity(*(uint *)(this + 0xa8));
  if (pOVar5 != (OSCollection *)0x0) {
    for (puVar12 = *(undefined8 **)(this + 0x140); puVar12 != (undefined8 *)0x0;
        puVar12 = (undefined8 *)puVar12[0xb]) {
      (**(code **)(*(long *)pOVar5 + 0x1c0))(pOVar5,*puVar12);
    }
    plVar6 = (long *)OSCollectionIterator::withCollection(pOVar5);
    if (plVar6 != (long *)0x0) {
      plVar7 = (long *)(**(code **)(*plVar6 + 0x128))(plVar6);
      while (plVar7 != (long *)0x0) {
        (**(code **)(*plVar7 + 0xa90))();
        plVar7 = (long *)(**(code **)(*plVar6 + 0x128))(plVar6);
      }
      (**(code **)(*plVar6 + 0x28))(plVar6);
      (**(code **)(*(long *)this + 0xaa8))(this);
      (**(code **)(*plVar9 + 0xa50))(plVar9);
      if (param_4) {
        local_a4 = (**(code **)(*plVar9 + 0x850))(plVar9,this,&local_48,&local_a0,local_b0);
        if (local_a4 == -0x1ffffd39) {
          local_a4 = (**(code **)(*plVar9 + 0xaf8))(plVar9,this,&local_48,local_b0);
        }
      }
      else {
        local_a4 = 0;
      }
      if (*(int *)(this + 0x150) == iVar11 + 1) {
        if (local_a4 == 0) {
          if ((local_68 != 0) && (local_60 != 0)) {
            (**(code **)(*(long *)this + 0xa28))(this);
          }
          plVar6 = (long *)IOAudioStream::createDictionaryFromFormat
                                     ((_IOAudioStreamFormat *)&local_48,
                                      (_IOAudioStreamFormatExtension *)&local_a0,(OSDictionary *)0x0
                                     );
          if (plVar6 == (long *)0x0) {
            local_a4 = -0x1ffffd44;
            goto LAB_00010fca;
          }
          if (*(long *)(this + 0xf8) != 0) {
            if ((char)local_38 == '\0') {
              (**(code **)(*(long *)this + 0x9f8))(this,0,0);
            }
            else if (((this[0x104] != (m_audio_b_FWStream)0x0) &&
                     (*(int *)(this + 0xb0) != (int)local_48)) &&
                    ((uVar10 = (int)local_48 * (int)plVar9[0x11], iVar11 = uVar10 * 4,
                     (uVar10 & 0x3fffffff) != 0 &&
                     (lVar8 = _IOMallocAligned(iVar11,0x20), lVar8 != 0)))) {
              (**(code **)(*(long *)this + 0x9f8))(this,lVar8,iVar11);
              this[0x104] = (m_audio_b_FWStream)0x1;
            }
          }
          iVar11 = *(int *)(this + 0xb0);
          *(undefined8 *)(this + 0xc0) = local_38;
          *(undefined8 *)(this + 0xb8) = local_40;
          *(undefined8 *)(this + 0xb0) = local_48;
          (**(code **)(*(long *)this + 0x268))(this,"IOAudioStreamFormat",plVar6);
          (**(code **)(*plVar6 + 0x28))(plVar6);
          if (*(int *)(this + 0xb0) != iVar11) {
            (**(code **)(*plVar9 + 0xa20))(plVar9);
          }
          if (local_b0 != (int *)0x0) {
            (**(code **)(*plVar9 + 0xb58))(plVar9,local_b0);
          }
        }
        else {
          _IOLog("IOAudioStream<0x%p>::setFormat(0x%p, 0x%p) - audio engine unable to change format\n"
                 ,this,param_1,param_3);
        }
        if (local_a4 == 0) {
          (**(code **)(*plVar9 + 0xba0))(plVar9,this);
        }
      }
LAB_00010fca:
      (**(code **)(*plVar9 + 0xa58))(plVar9);
      (**(code **)(*(long *)this + 0xab0))(this);
      plVar9 = (long *)OSCollectionIterator::withCollection(pOVar5);
      if (plVar9 == (long *)0x0) {
        local_a4 = -0x1ffffd43;
      }
      else {
        plVar6 = (long *)(**(code **)(*plVar9 + 0x128))(plVar9);
        while (plVar6 != (long *)0x0) {
          (**(code **)(*plVar6 + 0xa98))();
          plVar6 = (long *)(**(code **)(*plVar9 + 0x128))(plVar9);
        }
        (**(code **)(*plVar9 + 0x28))(plVar9);
      }
      (**(code **)(*(long *)pOVar5 + 0x28))(pOVar5);
      return local_a4;
    }
    (**(code **)(*(long *)pOVar5 + 0x28))(pOVar5);
  }
  return -0x1ffffd43;
}
