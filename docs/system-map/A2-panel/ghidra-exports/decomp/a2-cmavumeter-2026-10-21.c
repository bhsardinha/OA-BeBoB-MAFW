openjdk version "21.0.12.1" 2026-08-18 LTS
OpenJDK Runtime Environment Temurin-21.0.12.1+1 (build 21.0.12.1+1-LTS)
OpenJDK 64-Bit Server VM Temurin-21.0.12.1+1 (build 21.0.12.1+1-LTS, mixed mode)
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/DecompilerDependent/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Debug/Debugger/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/PyGhidra/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Processors/JVM/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/GnuDemangler/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Processors/PIC/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Debug/Debugger-rmi-trace/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/VersionTracking/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/BytePatterns/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/PDB/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/SwiftDemangler/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/Base/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/FileFormats/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/SystemEmulation/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/MicrosoftCodeAnalyzer/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Processors/DATA/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/WildcardAssembler/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Processors/Atmel/ghidra_scripts
    /Users/bernardosardinha/Documents/openaudio-fw1814/docs/system-map/A1-ghidra/scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/BSim/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Processors/8051/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/Decompiler/ghidra_scripts
    /Users/bernardosardinha/tools/ghidra_12.1.4_PUBLIC/Ghidra/Features/FunctionID/ghidra_scripts (HeadlessAnalyzer)  
=== CMAVUMeter::CMAVUMeter @ 000c81de === (GhidraScript)  
/* CMAVUMeter::CMAVUMeter(LStream*) */
void __thiscall CMAVUMeter::CMAVUMeter(CMAVUMeter *this,LStream *param_1)
{
  CMAVUMeter *pCVar1;
  uint uVar2;
  short sVar3;
  undefined4 uVar4;
  CMAGraphicImportersList *this_00;
  CMAOffScreenList *this_01;
  uint local_14;
  LAttachable::LAttachable((LAttachable *)(this + 0xf4));
  LView::LView((LView *)this,(LStream *)&PTR_PTR_001b7534);
  CMAControlMixin::CMAControlMixin((CMAControlMixin *)(this + 0x88));
  *(undefined ***)this = &PTR__CMAVUMeter_001b72ec;
  *(undefined ***)(this + 0xf4) = &PTR__CMAVUMeter_001b750c;
  *(undefined ***)(this + 0x88) = &PTR__CMAVUMeter_001b74e4;
  pCVar1 = this + 0x94;
  LStream::ReadBlock(param_1,pCVar1,2);
  *(ushort *)pCVar1 = *(ushort *)pCVar1 << 8 | *(ushort *)pCVar1 >> 8;
  LStream::ReadBlock(param_1,&local_14,4);
  local_14 = local_14 >> 0x18 | (local_14 & 0xff0000) >> 8 | (local_14 & 0xff00) << 8 |
             local_14 << 0x18;
  *(float *)(this + 0x98) = (float)(int)local_14;
  *(float *)(this + 0x9c) = (float)(int)local_14;
  LStream::ReadBlock(param_1,this + 0xa8,4);
  uVar2 = *(uint *)(this + 0xa8);
  *(uint *)(this + 0xa8) =
       uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8 | uVar2 << 0x18;
  LStream::ReadBlock(param_1,this + 0xac,4);
  uVar2 = *(uint *)(this + 0xac);
  *(uint *)(this + 0xac) =
       uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8 | uVar2 << 0x18;
  LStream::ReadBlock(param_1,this + 0xb0,4);
  uVar2 = *(uint *)(this + 0xb0);
  *(uint *)(this + 0xb0) =
       uVar2 >> 0x18 | (uVar2 & 0xff0000) >> 8 | (uVar2 & 0xff00) << 8 | uVar2 << 0x18;
  LStream::ReadPString(param_1,(uchar *)(this + 0xb4));
  LStream::ReadBlock(param_1,this + 0xd4,1);
  *(float *)(this + 0xa0) = (float)*(int *)(this + 0xac) + DAT_000f9b94;
  *(undefined4 *)(this + 0xd8) = 0;
  *(int *)(this + 0xdc) = (int)*(float *)(this + 0x98);
  *(int *)(this + 0xe0) = (int)*(float *)(this + 0x98);
  uVar4 = _TickCount();
  *(undefined4 *)(this + 0xa4) = uVar4;
  *(undefined4 *)(this + 0xe4) = 0;
  *(undefined4 *)(this + 0xe8) = *(undefined4 *)(this + 4);
  *(undefined4 *)(this + 0xec) = 0;
  *(undefined4 *)(this + 0xf0) = 0;
  if (sVUGraphicList == (CMAGraphicImportersList *)0x0) {
    this_00 = operator_new(0x28);
    CMAGraphicImportersList::CMAGraphicImportersList(this_00);
    sVUGraphicList = this_00;
    if (this_00 == (CMAGraphicImportersList *)0x0) {
      UDebugging::DebugThrow
                (0x6e696c50,"CMAVUMeter",
                 (uchar *)
                 "w/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/CMAControls/CMAVUMeter.cp"
                 ,0x6b);
    }
  }
  sVar3 = CMAGraphicImportersList::LoadGraphicImporter
                    (sVUGraphicList,*(long *)(this + 0xb0),(uchar *)(this + 0xb4),
                     (CMAGraphicImporterItem **)(this + 0xd8),false,false);
  if (sVar3 != 0) {
    UDebugging::DebugThrow
              ((int)sVar3,"CMAVUMeter",
               (uchar *)
               "w/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/CMAControls/CMAVUMeter.cp"
               ,0x6f);
  }
  if (sVUOffScreenList == (CMAOffScreenList *)0x0) {
    this_01 = operator_new(0x28);
    CMAOffScreenList::CMAOffScreenList(this_01);
    sVUOffScreenList = this_01;
    if (this_01 == (CMAOffScreenList *)0x0) {
      UDebugging::DebugThrow
                (0x6e696c50,"CMAVUMeter",
                 (uchar *)
                 "w/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/CMAControls/CMAVUMeter.cp"
                 ,0x75);
    }
  }
  this[0x8c] = (CMAVUMeter)0x1;
  return;
}
 (GhidraScript)  
=== CMAVUMeter::ReleaseOffScreens @ 000c8a1a === (GhidraScript)  
/* CMAVUMeter::ReleaseOffScreens() */
void __thiscall CMAVUMeter::ReleaseOffScreens(CMAVUMeter *this)
{
  if (*(int *)(this + 0xec) != 0) {
    CMAOffScreenList::ReleaseOffScreenItem(sVUOffScreenList,*(long *)(this + 0xb0));
    *(undefined4 *)(this + 0xec) = 0;
  }
  if (*(int *)(this + 0xf0) != 0) {
    CMAOffScreenList::ReleaseOffScreenItem
              (sVUOffScreenList,(long)(&DAT_00002710 + *(int *)(this + 0xb0)));
    *(undefined4 *)(this + 0xf0) = 0;
  }
  return;
}
 (GhidraScript)  
=== CMAVUMeter::FinishCreateSelf @ 000c8ad0 === (GhidraScript)  
/* CMAVUMeter::FinishCreateSelf() */
void __thiscall CMAVUMeter::FinishCreateSelf(CMAVUMeter *this)
{
  short sVar1;
  bool local_1d;
  Rect local_1c [12];
  LPane::FinishCreateSelf();
  CMAGraphicImporterItem::GetGraphicItemBounds(*(CMAGraphicImporterItem **)(this + 0xd8),local_1c);
  sVar1 = CMAOffScreenList::LoadOffScreenBuffer
                    (sVUOffScreenList,*(long *)(this + 0xb0),local_1c,(LGWorld **)(this + 0xec),
                     &local_1d);
  if (sVar1 != 0) {
    UDebugging::DebugThrow
              ((int)sVar1,"FinishCreateSelf",
               (uchar *)
               "w/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/CMAControls/CMAVUMeter.cp"
               ,0xa6);
  }
  if (local_1d != false) {
    UMAControlUtilities::DrawQTItemToBuffer
              (*(CMAGraphicImporterItem **)(this + 0xd8),*(LGWorld **)(this + 0xec),0x100,true,false
              );
  }
  sVar1 = CMAOffScreenList::LoadOffScreenBuffer
                    (sVUOffScreenList,(long)(&DAT_00002710 + *(int *)(this + 0xb0)),local_1c,
                     (LGWorld **)(this + 0xf0),&local_1d);
  if (sVar1 != 0) {
    UDebugging::DebugThrow
              ((int)sVar1,"FinishCreateSelf",
               (uchar *)
               "w/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/CMAControls/CMAVUMeter.cp"
               ,0xae);
  }
  if (local_1d != false) {
    UMAControlUtilities::DrawQTItemToBuffer
              (*(CMAGraphicImporterItem **)(this + 0xd8),*(LGWorld **)(this + 0xf0),0x100,true,true)
    ;
  }
  return;
}
 (GhidraScript)  
=== CMAVUMeter::SetValue @ 000c8c40 === (GhidraScript)  
/* CMAVUMeter::SetValue(long) */
void __thiscall CMAVUMeter::SetValue(CMAVUMeter *this,long param_1)
{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  uVar2 = _TickCount();
  fVar7 = *(float *)(this + 0x98);
  bVar1 = false;
  fVar8 = (float)param_1;
  if ((fVar7 != fVar8) || (NAN(fVar7) || NAN(fVar8))) {
    fVar5 = fVar8;
    if (fVar8 < fVar7) {
      fVar5 = fVar7 + DAT_000f9b98;
    }
    *(float *)(this + 0x98) = fVar5;
    fVar6 = (float)*(int *)(this + 0xac);
    fVar7 = fVar5;
    if (fVar6 < fVar5) {
      *(float *)(this + 0x98) = fVar6;
      fVar7 = fVar6;
    }
    fVar5 = (float)*(int *)(this + 0xa8);
    bVar1 = true;
    if (fVar7 < fVar5) {
      *(float *)(this + 0x98) = fVar5;
      fVar7 = fVar5;
    }
  }
  fVar5 = *(float *)(this + 0x9c);
  iVar4 = (int)fVar5;
  if (fVar8 <= fVar5) {
    if ((*(uint *)(this + 0xa4) < uVar2) && (fVar8 = (float)*(int *)(this + 0xa8), fVar8 < fVar5)) {
      fVar5 = fVar5 + DAT_000f9b9c;
      *(float *)(this + 0x9c) = fVar5;
      if (fVar5 < fVar8) {
        *(float *)(this + 0x9c) = fVar8;
        fVar5 = fVar8;
      }
    }
  }
  else {
    *(float *)(this + 0x9c) = fVar8;
    if (*(float *)(this + 0xa0) <= fVar8) {
      iVar3 = uVar2 + 0x3c;
    }
    else {
      iVar3 = uVar2 + 0x1e;
    }
    *(int *)(this + 0xa4) = iVar3;
    fVar5 = (float)*(int *)(this + 0xac);
    if (fVar5 < fVar8) {
      *(float *)(this + 0x9c) = fVar5;
      fVar8 = fVar5;
    }
    fVar6 = (float)*(int *)(this + 0xa8);
    fVar5 = fVar8;
    if (fVar8 < fVar6) {
      *(float *)(this + 0x9c) = fVar6;
      fVar5 = fVar6;
    }
  }
  if (!bVar1) {
    if (((float)iVar4 == fVar5) && (!NAN((float)iVar4) && !NAN(fVar5))) {
      return;
    }
  }
  DrawMeter(this,(int)fVar7,(int)fVar5);
  return;
}
 (GhidraScript)  
=== CMAVUMeter::DrawMeter @ 000c8d88 === (GhidraScript)  
/* CMAVUMeter::DrawMeter(long, long) */
void __thiscall CMAVUMeter::DrawMeter(CMAVUMeter *this,long param_1,long param_2)
{
  undefined *puVar1;
  char cVar2;
  OpaqueGrafPtr *pOVar3;
  Rect local_64 [8];
  Rect local_5c [8];
  StColorPortState local_54 [64];
  int local_14;
  puVar1 = PTR____stack_chk_guard_00163028;
  local_14 = *(int *)PTR____stack_chk_guard_00163028;
  pOVar3 = (OpaqueGrafPtr *)(**(code **)(*(int *)this + 0x14))(this);
  StColorPortState::StColorPortState(local_54,pOVar3);
  *(long *)(this + 0xdc) = param_1;
  *(long *)(this + 0xe0) = param_2;
  LView::OutOfFocus((LView *)this);
  cVar2 = (**(code **)(*(int *)this + 0x118))(this,0);
  if (cVar2 != '\0') {
    (**(code **)(*(int *)this + 0x4c))(this,local_5c);
    (**(code **)(*(int *)this + 0x48))(this,local_64);
    pOVar3 = (OpaqueGrafPtr *)(**(code **)(*(int *)this + 0x14))(this);
    DrawMeterToPort(this,pOVar3,local_5c,local_64);
    LView::OutOfFocus((LView *)0x0);
  }
  StColorPortState::~StColorPortState(local_54);
  if (*(int *)puVar1 == local_14) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}
 (GhidraScript)  
=== CMAVUMeter::SetValueImmediate @ 000c8e7a === (GhidraScript)  
/* CMAVUMeter::SetValueImmediate(long) */
void __thiscall CMAVUMeter::SetValueImmediate(CMAVUMeter *this,long param_1)
{
  int iVar1;
  int iVar2;
  float fVar3;
  float fVar4;
  fVar3 = (float)param_1;
  if ((*(float *)(this + 0x98) != fVar3) || (NAN(*(float *)(this + 0x98)) || NAN(fVar3))) {
    *(float *)(this + 0x98) = fVar3;
    fVar4 = (float)*(int *)(this + 0xac);
    if (fVar4 < fVar3) {
      *(float *)(this + 0x98) = fVar4;
      fVar3 = fVar4;
    }
    fVar4 = (float)*(int *)(this + 0xa8);
    if (fVar3 < fVar4) {
      *(float *)(this + 0x98) = fVar4;
      fVar3 = fVar4;
    }
    *(float *)(this + 0x9c) = fVar3;
    iVar1 = _TickCount();
    iVar2 = 0x3c;
    if (*(float *)(this + 0x9c) < *(float *)(this + 0xa0)) {
      iVar2 = 0x1e;
    }
    *(int *)(this + 0xa4) = iVar2 + iVar1;
    DrawMeter(this,(int)*(float *)(this + 0x98),(int)*(float *)(this + 0x9c));
  }
  return;
}
 (GhidraScript)  
=== CMAVUMeter::DrawSelf @ 000c9006 === (GhidraScript)  
/* CMAVUMeter::DrawSelf() */
void __thiscall CMAVUMeter::DrawSelf(CMAVUMeter *this)
{
  DrawMeter(this,(int)*(float *)(this + 0x98),(int)*(float *)(this + 0x9c));
  return;
}
 (GhidraScript)  
=== CMAVUMeter::DrawMeterToPort @ 000c9034 === (GhidraScript)  
/* CMAVUMeter::DrawMeterToPort(OpaqueGrafPtr*, Rect const&, Rect const&) */
void __thiscall
CMAVUMeter::DrawMeterToPort(CMAVUMeter *this,OpaqueGrafPtr *param_1,Rect *param_2,Rect *param_3)
{
  undefined4 uVar1;
  undefined4 uVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  char cVar6;
  short sVar7;
  CMAVUMeter *pCVar8;
  int iVar9;
  int iVar10;
  undefined4 *puVar11;
  uint *puVar12;
  int iVar13;
  int iVar14;
  short sVar15;
  short sVar16;
  int iVar17;
  int iVar18;
  longdouble lVar19;
  longdouble lVar20;
  ulonglong uVar21;
  float fVar22;
  float local_a0;
  float local_94;
  float local_90;
  undefined4 local_4c [2];
  short local_44;
  short local_40;
  CGContext *local_38;
  uint local_34;
  ushort local_30;
  uint local_2c;
  ushort local_28;
  undefined4 local_24;
  short local_20;
  short sStack_1e;
  undefined4 local_1c;
  undefined4 local_18;
  StColorState::Normalize();
  cVar6 = LPane::IsActive((LPane *)this);
  if ((cVar6 == '\0') || (cVar6 = LPane::IsEnabled((LPane *)this), cVar6 == '\0')) {
    pCVar8 = this + 0xf0;
  }
  else {
    pCVar8 = this + 0xec;
  }
  iVar14 = *(int *)(this + 0xac) - *(int *)(this + 0xa8);
  puVar11 = *(undefined4 **)pCVar8;
  uVar1 = puVar11[1];
  uVar2 = puVar11[2];
  local_1c = *(undefined4 *)param_3;
  sVar16 = (short)uVar1;
  sVar4 = (short)uVar2;
  sVar7 = sVar4 - sVar16;
  sVar5 = (short)((uint)uVar1 >> 0x10);
  sVar15 = (short)((uint)uVar2 >> 0x10);
  sVar3 = sVar15 - sVar5;
  local_18 = CONCAT22((short)((uint)local_1c >> 0x10) + sVar3,(short)local_1c + sVar7);
  local_24 = *(undefined4 *)param_2;
  _local_20 = CONCAT22((short)((uint)local_24 >> 0x10) + sVar3,(short)local_24 + sVar7);
  iVar9 = (int)sVar3;
  UMAControlUtilities::DrawBackgroundBehindPaneToPort((LPane *)this,param_3,param_1,param_2);
  iVar17 = (int)sVar7;
  iVar13 = *(int *)(this + 0xa8);
  iVar18 = *(int *)(this + 0xdc);
  iVar10 = iVar18 - iVar13;
  if (iVar10 != 0 && iVar13 <= iVar18) {
    local_1c = uVar1;
    local_18 = uVar2;
    if (iVar18 < *(int *)(this + 0xac)) {
      if (this[0xd4] == (CMAVUMeter)0x0) {
        if (*(short *)(this + 0x94) != 0) {
          sVar16 = sVar4 - (short)((iVar10 * iVar17) / iVar14);
          goto LAB_000c9249;
        }
        sVar15 = (short)((iVar10 * iVar9) / iVar14) + sVar5;
        local_18 = CONCAT22(sVar15,sVar4);
      }
      else if (*(short *)(this + 0x94) == 0) {
        uVar21 = (ulonglong)(double)iVar13 & DAT_000f9af0;
        lVar19 = (longdouble)_log((double)((ulonglong)(double)iVar18 & DAT_000f9af0) + DAT_000f9028)
        ;
        lVar20 = (longdouble)_log(uVar21);
        sVar15 = (short)(int)((double)(int)sVar5 +
                             ((double)iVar9 - ((double)lVar19 / (double)lVar20) * (double)iVar9));
        local_18 = CONCAT22(sVar15,(undefined2)local_18);
      }
      else {
        uVar21 = (ulonglong)(double)iVar13 & DAT_000f9af0;
        lVar19 = (longdouble)_log((double)((ulonglong)(double)iVar18 & DAT_000f9af0) + DAT_000f9028)
        ;
        lVar20 = (longdouble)_log(uVar21);
        sVar16 = (short)(int)((double)(int)sVar4 -
                             ((double)iVar17 - ((double)lVar19 / (double)lVar20) * (double)iVar17));
LAB_000c9249:
        local_1c = CONCAT22(local_1c._2_2_,sVar16);
      }
    }
    sVar3 = (short)((uint)*(undefined4 *)param_2 >> 0x10);
    sVar7 = (short)*(undefined4 *)param_2 + sVar7;
    _local_20 = CONCAT22((sVar3 + sVar15) - sVar5,sVar7);
    local_24 = CONCAT22(sVar3,sVar16 + (sVar7 - sVar4));
    puVar11 = (undefined4 *)_GetPortPixMap(*puVar11);
    uVar1 = *puVar11;
    puVar11 = (undefined4 *)_GetPortPixMap(param_1);
    _CopyBits(uVar1,*puVar11,&local_1c,&local_24,0,0);
    iVar13 = *(int *)(this + 0xa8);
  }
  if (*(int *)(this + 0xe0) <= iVar13 + 1) {
    return;
  }
  puVar12 = &Color_Red;
  if ((float)*(int *)(this + 0xe0) < *(float *)(this + 0xa0)) {
    puVar12 = (uint *)PTR__Color_Black_00163044;
  }
  local_28 = (ushort)puVar12[1];
  local_2c = *puVar12;
  cVar6 = LPane::IsEnabled((LPane *)this);
  if ((cVar6 == '\0') || (cVar6 = LPane::IsActive((LPane *)this), cVar6 == '\0')) {
    UGraphicUtils::Lighten((UGraphicUtils *)&local_34,(RGBColor *)&local_2c);
    local_28 = local_30;
    local_2c = local_34;
  }
  iVar13 = _QDBeginCGContext(param_1);
  if (iVar13 != 0) {
    return;
  }
  StRegion::StRegion((StRegion *)local_4c);
  _CGContextSetShouldAntialias(local_38);
  _CGContextSetLineCap(local_38);
  _CGContextSetLineWidth(local_38);
  _CGContextSetRGBStrokeColor
            (local_38,(float)(local_2c & 0xffff) / DAT_000f9594,
             (float)(local_2c >> 0x10) / DAT_000f9594,(float)local_28 / DAT_000f9594,0x3f800000);
  _GetPortBounds(param_1);
  iVar18 = (int)local_20;
  iVar13 = (int)local_24._2_2_;
  _GetClip(local_4c[0]);
  UQuartzUtils::ClipContextToPane(local_38,(LPane *)this,param_3,(StRegion *)local_4c);
  uVar21 = DAT_000f9af0;
  fVar22 = (float)((short)(local_40 - local_44) - iVar18);
  local_94 = (float)iVar13;
  iVar13 = *(int *)(this + 0xac);
  iVar18 = *(int *)(this + 0xe0);
  if (this[0xd4] == (CMAVUMeter)0x0) {
    if (*(short *)(this + 0x94) == 0) {
      if (iVar18 < iVar13) {
        local_94 = local_94 + (float)(((iVar18 - *(int *)(this + 0xa8)) * iVar9) / iVar14);
      }
      else {
        local_94 = (float)iVar9 + local_94 + DAT_000f9b94;
      }
      local_a0 = (float)iVar17 + fVar22;
      local_90 = local_94;
      goto LAB_000c9844;
    }
    if (iVar18 < iVar13) {
      local_a0 = fVar22 + (float)(((iVar18 - *(int *)(this + 0xa8)) * iVar17) / iVar14);
    }
    else {
      local_a0 = (float)iVar17 + fVar22 + DAT_000f9b94;
    }
  }
  else {
    if (*(short *)(this + 0x94) == 0) {
      if (iVar18 < iVar13) {
        lVar19 = (longdouble)_log((double)((ulonglong)(double)iVar18 & DAT_000f9af0) + DAT_000f9028)
        ;
        lVar20 = (longdouble)_log((ulonglong)(double)*(int *)(this + 0xa8) & uVar21);
        local_94 = local_94 +
                   (float)(int)((double)(iVar9 + 1) -
                               ((double)lVar19 / (double)lVar20) * (double)iVar9);
      }
      else {
        local_94 = (float)iVar9 + local_94 + DAT_000f9b94;
      }
      local_a0 = (float)iVar17 + fVar22;
      local_90 = local_94;
      goto LAB_000c9844;
    }
    if (iVar18 < iVar13) {
      lVar19 = (longdouble)_log((double)((ulonglong)(double)iVar18 & DAT_000f9af0) + DAT_000f9028);
      lVar20 = (longdouble)_log((ulonglong)(double)*(int *)(this + 0xa8) & uVar21);
      local_a0 = fVar22 + (float)(int)((double)(iVar17 + 1) -
                                      ((double)lVar19 / (double)lVar20) * (double)iVar17);
    }
    else {
      local_a0 = (float)iVar17 + fVar22 + DAT_000f9b94;
    }
  }
  local_90 = (float)iVar9 + local_94;
  fVar22 = local_a0;
LAB_000c9844:
  _CGContextBeginPath(local_38);
  _CGContextMoveToPoint(local_38,local_94,fVar22);
  _CGContextAddLineToPoint(local_38,local_90,local_a0);
  _CGContextClosePath(local_38);
  _CGContextStrokePath(local_38);
  _QDEndCGContext(param_1);
  StRegion::~StRegion((StRegion *)local_4c);
  return;
}
 (GhidraScript)  
=== CMAVUMeter::HideSelf @ 000c98f6 === (GhidraScript)  
/* CMAVUMeter::HideSelf() */
void __thiscall CMAVUMeter::HideSelf(CMAVUMeter *this)
{
  float fVar1;
  LPane::HideSelf();
  fVar1 = (float)*(int *)(this + 0xa8);
  *(float *)(this + 0x9c) = fVar1;
  *(float *)(this + 0x98) = fVar1;
  *(int *)(this + 0xe0) = (int)fVar1;
  *(int *)(this + 0xdc) = (int)fVar1;
  return;
}
 (GhidraScript)  
