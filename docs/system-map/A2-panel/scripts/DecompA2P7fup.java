import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P7fup extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // The translator quartet
            "0006f436", // CXDevice::SetHWSetting           (1851 B)
            "000704bc", // CXDevice::SetHWIndexedSetting    (2450 B)
            // Kext->UI listener registration (confirms wiring)
            "0006ca12", // CXDevice::InstallPropertyChangeHandlers (already read in P3 - re-read for xref count)
            // Sample Receive handlers (prove inverse mapping)
            "00071a9e", // ReceiveSampleRate   (MASampleRate → cookie)
            "00071b40", // ReceiveClockSource  (MAClockSource → cookie)
            "00071b6c"  // ReceiveOnePropertyChange (dispatcher — the inverse of the translator switch)
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 240, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
