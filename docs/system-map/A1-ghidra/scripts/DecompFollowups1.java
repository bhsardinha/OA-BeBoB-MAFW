import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompFollowups1 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            "0000dd3e", // FW1814Device::SetInputPan
            "0000dbea", // FW1814Device::SetInputRouting
            "0000cfc4", // FW1814Device::SetAuxMonitor
            "0001f300", // BaseChannelGroup::AdjustedLevels
            "000080cc", // OSArrayToSeriatimValues
            "0000824e", // SeriatimValuesToOSArray
            "00018884", // ClipSubEngineOutput
            "000188e4", // ConvertSubEngineInput
            "00018c44", // ActivateSubEngine
            "00018b1e", // DeactivateSubEngine
            "00007492", // ActionStartMIDI
            "00007574", // ActionSendMIDI
            "0000717c", // CoreStartMIDI
            "0000714a", // CoreStopMIDI
            "0001d80e", // HSCIReadComplete
            "0001cc9e", // FWMetaNub::FirmwareCacheInfoRegisters
            "00022e28", // FWAVCAudioDevice::setProperties
            "0001185e", // StartAC3Output
            "000076de", // FWUserClient::start
            "00007220"  // FWUserClient::clientMemoryForType
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 90, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
