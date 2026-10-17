// m_audio_b_FWDCLInputProgram::InputDCLStaticCallback
// Entry: 00025abe  Size: 37 bytes
// Prototype: undefined InputDCLStaticCallback(DCLCommandStruct * param_1)


/* m_audio_b_FWDCLInputProgram::InputDCLStaticCallback(DCLCommandStruct*) */

void m_audio_b_FWDCLInputProgram::InputDCLStaticCallback(DCLCommandStruct *param_1)

{
  undefined8 *puVar1;
  
  if (((param_1 != (DCLCommandStruct *)0x0) &&
      (puVar1 = *(undefined8 **)(param_1 + 0x20), puVar1 != (undefined8 *)0x0)) &&
     ((m_audio_b_FWDCLInputProgram *)*puVar1 != (m_audio_b_FWDCLInputProgram *)0x0)) {
    InputDCLCallbackCopyPacketGroupToSamples
              ((m_audio_b_FWDCLInputProgram *)*puVar1,*(uint *)(puVar1 + 1));
    return;
  }
  return;
}


