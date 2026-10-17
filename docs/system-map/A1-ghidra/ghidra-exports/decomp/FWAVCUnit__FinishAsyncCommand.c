// m_audio_b_FWAVCUnit::FinishAsyncCommand
// Entry: 000016c8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::FinishAsyncCommand(m_audio_b_FWAsyncAVCCommand*) */

undefined8 __thiscall
m_audio_b_FWAVCUnit::FinishAsyncCommand
          (m_audio_b_FWAVCUnit *this,m_audio_b_FWAsyncAVCCommand *param_1)

{
  m_audio_b_FWAsyncAVCCommand *pmVar1;
  IOFireWireAVCNub *pIVar2;
  
  pmVar1 = *(m_audio_b_FWAsyncAVCCommand **)(this + 0xe8);
  if ((pmVar1 == param_1) && (pmVar1 != (m_audio_b_FWAsyncAVCCommand *)0x0)) {
    pIVar2 = *(IOFireWireAVCNub **)(pmVar1 + 0x98);
    *(IOFireWireAVCNub **)(this + 0xe8) = pIVar2;
    if (*(m_audio_b_FWAsyncAVCCommand **)(this + 0xf0) == param_1) {
      *(undefined8 *)(this + 0xf0) = 0;
    }
    if (pIVar2 != (IOFireWireAVCNub *)0x0) {
      IOFireWireAVCAsynchronousCommand::submit(pIVar2);
    }
    (**(code **)(*(long *)param_1 + 0x28))(param_1);
  }
  return 0;
}
