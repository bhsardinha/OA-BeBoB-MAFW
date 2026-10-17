import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;

public class DecompQ13 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] wants = {
            "com_m_audio_FWEngine2::com_m_audio_FWEngine2",
            "com_m_audio_FWEngine2::MetaClass::alloc",
            "com_m_audio_FWAudiophileDevice::SetupAudioEngine",
            "com_m_audio_FW1814Device::SetupAudioEngine",
            "m_audio_b_FWPairEngine::performAudioEngineStart",
            "m_audio_b_FWDiscreteEngine::performAudioEngineStart",
            "m_audio_b_FWDiscreteEngine::clipOutputSamples",
            "m_audio_b_FWPairEngine::clipOutputSamples"
        };
        for (String w : wants) {
            for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
                if (f.getName(true).equals(w)) {
                    DecompileResults r = di.decompileFunction(f, 60, monitor);
                    println("=== " + w + " @ " + f.getEntryPoint() + " ===");
                    println(r.getDecompiledFunction().getC());
                    break;
                }
            }
        }
    }
}
