import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompClockBijection extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            "00022e28", // m_audio_b_FWAVCAudioDevice::setProperties
            "00023858", // m_audio_b_FWAVCAudioDevice::SetClockSource
            "00023148", // m_audio_b_FWAVCAudioDevice::SetClockSourceInternal (parent)
            "0000e25c", // com_m_audio_FW1814Device::SetClockSourceInternal
            "0000e4f0"  // com_m_audio_FW1814Device::SetBlankSlateClockSource
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 180, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
