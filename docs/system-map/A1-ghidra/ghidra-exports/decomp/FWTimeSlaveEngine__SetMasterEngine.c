// m_audio_b_FWTimeSlaveEngine::SetMasterEngine
// Entry: 00019e8a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWTimeSlaveEngine::SetMasterEngine(m_audio_b_FWDiscreteEngine*) */

void __thiscall
m_audio_b_FWTimeSlaveEngine::SetMasterEngine
          (m_audio_b_FWTimeSlaveEngine *this,m_audio_b_FWDiscreteEngine *param_1)

{
  long lVar1;
  long *plVar2;
  undefined8 uVar3;
  
  lVar1 = *(long *)(this + 0x110);
  *(m_audio_b_FWDiscreteEngine **)(this + 0x118) = param_1;
  plVar2 = (long *)m_audio_b_FWDiscreteEngine::GetStatusDescriptor(param_1);
  *(long **)(this + 0x120) = plVar2;
  (**(code **)(*plVar2 + 0x20))(plVar2);
  if (lVar1 != 0) {
    (**(code **)(**(long **)(lVar1 + 8) + 0x28))();
    *(undefined8 *)(lVar1 + 8) = *(undefined8 *)(this + 0x120);
  }
  uVar3 = (**(code **)(**(long **)(this + 0x120) + 0x2e0))();
  *(undefined8 *)(this + 0xb0) = uVar3;
                    /* WARNING: Could not recover jumptable at 0x00019f09. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xb40))(this,*(undefined4 *)(*(long *)(this + 0x118) + 0x88));
  return;
}
