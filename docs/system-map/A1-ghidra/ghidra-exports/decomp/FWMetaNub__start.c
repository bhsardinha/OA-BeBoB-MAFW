// com_m_audio_FWMetaNub::start
// Entry: 0001cf68
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::start(IOService*) */

undefined8 __thiscall com_m_audio_FWMetaNub::start(com_m_audio_FWMetaNub *this,IOService *param_1)

{
  char cVar1;
  int iVar2;
  long lVar3;
  long *plVar4;
  char *pcVar5;
  OSMetaClassBase *pOVar6;
  undefined8 uVar7;
  ushort local_6e;
  ushort local_6c [2];
  char local_68 [66];
  ushort local_26;
  ushort local_24 [2];
  
  cVar1 = (**(code **)(PTR_vtable_000300e0 + 0x5d0))(this,param_1);
  if (cVar1 == '\0') {
    return 0;
  }
  if (param_1 != (IOService *)0x0) {
    lVar3 = OSMetaClassBase::safeMetaCast
                      ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030050);
    *(long *)(this + 0xe0) = lVar3;
    if (lVar3 != 0) {
      lVar3 = IOFireWireNub::getController();
      *(long *)(this + 0xe8) = lVar3;
      if (lVar3 != 0) {
        if (*(uint **)(this + 0xe0) != (uint *)0x0) {
          IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_24,&local_26);
        }
        CopyPropertyFrom(this,param_1,"FireWire Product Name");
        CopyPropertyFrom(this,param_1,"FireWire Vendor Name");
        CopyPropertyFrom(this,param_1,"Vendor_ID");
        CopyPropertyFrom(this,param_1,"Model_ID");
        CopyPropertyFrom(this,param_1,"GUID");
        plVar4 = (long *)(**(code **)(*(long *)this + 0x2b8))(this,"FireWire Product Name");
        pcVar5 = (char *)(**(code **)(*plVar4 + 0x148))(plVar4);
        lVar3 = GetActualProductName(this,pcVar5);
        if (lVar3 != 0) {
          (**(code **)(*(long *)this + 0x270))(this,"MAActualProductName",lVar3);
          iVar2 = FirmwareCacheInfoRegisters((IOService *)this);
          if (iVar2 == 0) {
            pOVar6 = (OSMetaClassBase *)(**(code **)(*(long *)this + 0x2b8))(this,"GUID");
            plVar4 = (long *)OSMetaClassBase::safeMetaCast
                                       (pOVar6,*(OSMetaClass **)PTR_metaClass_00030080);
            builtin_strncpy(local_68,"<unknown>",10);
            local_68[0x38] = '\0';
            local_68[0x39] = '\0';
            local_68[0x3a] = '\0';
            local_68[0x3b] = '\0';
            local_68[0x3c] = '\0';
            local_68[0x3d] = '\0';
            local_68[0x3e] = '\0';
            local_68[0x3f] = '\0';
            local_68[0x30] = '\0';
            local_68[0x31] = '\0';
            local_68[0x32] = '\0';
            local_68[0x33] = '\0';
            local_68[0x34] = '\0';
            local_68[0x35] = '\0';
            local_68[0x36] = '\0';
            local_68[0x37] = '\0';
            local_68[0x28] = '\0';
            local_68[0x29] = '\0';
            local_68[0x2a] = '\0';
            local_68[0x2b] = '\0';
            local_68[0x2c] = '\0';
            local_68[0x2d] = '\0';
            local_68[0x2e] = '\0';
            local_68[0x2f] = '\0';
            local_68[0x20] = '\0';
            local_68[0x21] = '\0';
            local_68[0x22] = '\0';
            local_68[0x23] = '\0';
            local_68[0x24] = '\0';
            local_68[0x25] = '\0';
            local_68[0x26] = '\0';
            local_68[0x27] = '\0';
            local_68[0x18] = '\0';
            local_68[0x19] = '\0';
            local_68[0x1a] = '\0';
            local_68[0x1b] = '\0';
            local_68[0x1c] = '\0';
            local_68[0x1d] = '\0';
            local_68[0x1e] = '\0';
            local_68[0x1f] = '\0';
            local_68[0x10] = '\0';
            local_68[0x11] = '\0';
            local_68[0x12] = '\0';
            local_68[0x13] = '\0';
            local_68[0x14] = '\0';
            local_68[0x15] = '\0';
            local_68[0x16] = '\0';
            local_68[0x17] = '\0';
            local_68[10] = '\0';
            local_68[0xb] = '\0';
            local_68[0xc] = '\0';
            local_68[0xd] = '\0';
            local_68[0xe] = '\0';
            local_68[0xf] = '\0';
            if (plVar4 != (long *)0x0) {
              uVar7 = (**(code **)(*plVar4 + 0x150))(plVar4);
              _snprintf(local_68,0x40,"%llx",uVar7);
            }
            (**(code **)(*(long *)this + 0x270))(this,"GUIDString",local_68);
            _IOLog("M-FW: metanub 0x%p GUID:%s\n",this,local_68);
            *(undefined4 *)(this + 0x88) = 1;
            (**(code **)(*(long *)this + 0x5b0))(this,0);
            uVar7 = 1;
            goto LAB_0001d188;
          }
        }
      }
    }
  }
  uVar7 = 0;
LAB_0001d188:
  if (*(uint **)(this + 0xe0) != (uint *)0x0) {
    IOFireWireNub::getNodeIDGeneration(*(uint **)(this + 0xe0),local_6c,&local_6e);
  }
  return uVar7;
}
