// com_m_audio_FWMetaNub::DownloadImageStart
// Entry: 0001c5d0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::DownloadImageStart() */

void __thiscall com_m_audio_FWMetaNub::DownloadImageStart(com_m_audio_FWMetaNub *this)

{
  long lVar1;
  
  lVar1 = *(long *)(this + 0x140);
  DownloadSectionStart
            (this,0,*(uint *)(lVar1 + 0x34),*(uint *)(lVar1 + 0x38),*(uint *)(lVar1 + 0x3c),
             *(uint *)(lVar1 + 0x30));
  return;
}
