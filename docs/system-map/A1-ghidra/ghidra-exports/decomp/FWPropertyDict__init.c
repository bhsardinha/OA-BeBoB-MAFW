// m_audio_b_FWPropertyDict::init
// Entry: 000099a4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::init(IOService*) */

undefined1 __thiscall
m_audio_b_FWPropertyDict::init(m_audio_b_FWPropertyDict *this,IOService *param_1)

{
  undefined1 uVar1;
  char cVar2;
  long *plVar3;
  
  if ((param_1 == (IOService *)0x0) ||
     (cVar2 = (**(code **)(PTR_vtable_000300c0 + 0x128))(this,param_1,0), cVar2 == '\0')) {
    uVar1 = 0;
  }
  else {
    *(IOService **)(this + 0x48) = param_1;
    *(undefined8 *)(this + 0x60) = 0;
    *(undefined8 *)(this + 0x58) = 0;
    *(undefined8 *)(this + 0x50) = 0;
    plVar3 = (long *)(**(code **)(*(long *)param_1 + 0x688))(param_1);
    *(long **)(this + 0x68) = plVar3;
    (**(code **)(*plVar3 + 0x20))(plVar3);
    plVar3 = *(long **)(this + 0x68);
    if (plVar3 == (long *)0x0) {
      uVar1 = 1;
    }
    else {
      (**(code **)(*plVar3 + 0x140))(plVar3,this);
      uVar1 = 1;
    }
  }
  return uVar1;
}
