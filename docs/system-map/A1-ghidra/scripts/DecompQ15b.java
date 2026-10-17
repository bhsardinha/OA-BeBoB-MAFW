import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompQ15b extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            "0000299c", // m_audio_b_FWAudioDevice::setProperties (thunk candidate)
            "00022e28", // m_audio_b_FWAVCAudioDevice::setProperties (MASampleRate + fallthrough)
            "0000a0e0", // m_audio_b_FWPropertyDict::SetProperties (workhorse)
            "0001cb64"  // com_m_audio_FWMetaNub::setProperties
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 120, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
