// com_m_audio_FWMetaNub::DownloadCNEStart
// Entry: 0001c5ac
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::DownloadCNEStart() */

void __thiscall com_m_audio_FWMetaNub::DownloadCNEStart(com_m_audio_FWMetaNub *this)

{
  long lVar1;
  
  lVar1 = *(long *)(this + 0x140);
  DownloadSectionStart
            (this,1,0,*(uint *)(lVar1 + 0x58),*(uint *)(lVar1 + 0x5c),*(uint *)(lVar1 + 0x50));
  return;
}
