// m_audio_FWb::BaseChannelGroup::SetupBatchChangeMethod<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
// Entry: 00014f64
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* void m_audio_FWb::BaseChannelGroup::SetupBatchChangeMethod<com_m_audio_FW410Device*, int
   (m_audio_b_FWAVCAudioDevice::*)(unsigned int, unsigned int, void*)>(com_m_audio_FW410Device*, int
   (m_audio_b_FWAVCAudioDevice::*)(unsigned int, unsigned int, void*), unsigned int, int) */

void m_audio_FWb::BaseChannelGroup::
     SetupBatchChangeMethod<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*)>
               (com_m_audio_FW410Device *param_1,_func_int_uint_uint_void_ptr *param_2,uint param_3,
               int param_4)

{
  undefined8 *puVar1;
  
  *(undefined4 *)(param_1 + 0x44) = 0x80;
  *(uint *)(param_1 + 0x40) = param_3;
  if (*(void **)(param_1 + 0x38) != (void *)0x0) {
    operator_delete(*(void **)(param_1 + 0x38));
  }
  if (param_2 == (_func_int_uint_uint_void_ptr *)0x0) {
    puVar1 = (undefined8 *)0x0;
  }
  else {
    puVar1 = operator_new(0x20);
    if (puVar1 != (undefined8 *)0x0) {
      *puVar1 = &PTR_Call_0003c4e0;
      puVar1[1] = param_2;
      puVar1[2] = 0xba9;
      puVar1[3] = 0;
    }
  }
  *(undefined8 **)(param_1 + 0x38) = puVar1;
  return;
}
