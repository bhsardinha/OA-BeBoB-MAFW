// m_audio_b_FWRemoteIsochPort::releasePort
// Entry: 00018416
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWRemoteIsochPort::releasePort() */

void __thiscall m_audio_b_FWRemoteIsochPort::releasePort(m_audio_b_FWRemoteIsochPort *this)

{
  code *UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE = *(code **)(**(long **)(this + 0x10) + 0x128);
                    /* WARNING: Could not recover jumptable at 0x0001843b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)
            (*(long **)(this + 0x10),this[0x18] == (m_audio_b_FWRemoteIsochPort)0x0,0,
             *(undefined4 *)(this + 0x1c),UNRECOVERED_JUMPTABLE);
  return;
}
