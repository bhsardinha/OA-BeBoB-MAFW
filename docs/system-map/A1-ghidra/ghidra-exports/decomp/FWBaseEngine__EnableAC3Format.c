// m_audio_b_FWBaseEngine::EnableAC3Format
// Entry: 0000445e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* m_audio_b_FWBaseEngine::EnableAC3Format() */

void __thiscall m_audio_b_FWBaseEngine::EnableAC3Format(m_audio_b_FWBaseEngine *this)

{
  uint uVar1;
  
  if ((this[0x2d4] == (m_audio_b_FWBaseEngine)0x0) && (this[0x238] == (m_audio_b_FWBaseEngine)0x0))
  {
    if (this[0x2d3] == (m_audio_b_FWBaseEngine)0x0) {
      uVar1 = *(uint *)(this + 0x344);
    }
    else {
      (**(code **)(*(long *)this + 0xcc0))(this);
      uVar1 = *(uint *)(this + 0x344) | 0x8000;
      *(uint *)(this + 0x344) = uVar1;
    }
    this[0x2d4] = (m_audio_b_FWBaseEngine)0x1;
    *(undefined4 *)(this + 0x2ec) = 2;
    *(uint *)(this + 0x344) = uVar1 | 0x270000;
                    /* WARNING: Could not recover jumptable at 0x000044e3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(long **)(this + 0x138) + 0xb38))(*(long **)(this + 0x138),1);
    return;
  }
  return;
}
