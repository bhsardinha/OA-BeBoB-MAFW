import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompQ15 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            "0000eec0", // FWSettings1814Levels ctor
            "0000efda", // FWSettings1814AuxSend ctor
            "0000d128", // SetInputPan
            "0000d15c", // SetInputRouting
            "0000d0b4", // SetAuxMonitor
            "000093a4", // FWChannelGroupProperty::init
            "0000940e", // FWChannelGroupProperty::CallSetter
            "0000e184", // BaseChannelGroup::SetupBatchChangeMethod<FW1814,...>
            "0000b3b4", // ConcreteChannelGroup<6>::ctor
            "0000b432", // ConcreteChannelGroup<2>::ctor
            "0000bd5a", // FWSettingsLevels ctor (base)
            "0000bd1c", // ChannelGroupProperties ctor
            "0000ac9e"  // ConcreteChannelGroup<2>::ChannelAtIndex
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
