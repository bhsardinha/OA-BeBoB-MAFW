// com_m_audio_FWMetaNub::StateString
// Entry: 0001b81e  Size: 95 bytes
// Prototype: undefined StateString(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::StateString() */

char * __thiscall com_m_audio_FWMetaNub::StateString(com_m_audio_FWMetaNub *this)

{
  switch(*(undefined4 *)(this + 0x88)) {
  case 0:
    return "nub start";
  case 1:
    return "initial-register";
  case 2:
    return "reregister";
  case 3:
    return "firmware-start";
  case 4:
    return "firmware-halt";
  case 5:
    return "firmware-update";
  default:
    return "<unknown>";
  }
}


