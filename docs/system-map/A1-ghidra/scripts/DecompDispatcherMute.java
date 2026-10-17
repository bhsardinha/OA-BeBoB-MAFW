import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompDispatcherMute extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            "0000e5dc", // com_m_audio_FW1814Device::SwitchDigitalSignals (bit4/bit5 handler candidate)
            "0000e6ca", // com_m_audio_FW1814Device::InitializeUserSettableProperties
            "00022e28", // m_audio_b_FWAVCAudioDevice::setProperties (parent dispatcher)
            "00023858", // m_audio_b_FWAVCAudioDevice::SetClockSource (legacy translator)
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
