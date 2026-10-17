// A3-M9 — sleep/wake path.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DecompA3M9 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m9.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        String[] addrs = {
            "0x000042c5",  // MAFWBeBoBPowerNotification::observeSleepNotifications:
            "0x00004245",  // MAFWBeBoBPowerNotification::stopObserving
            "0x00002468",  // MAFWMIDIDeviceBeBoB::systemWillSleep (variant 1)
            "0x000024ba",  // MAFWMIDIDeviceBeBoB::systemWillSleep (variant 2)
            "0x00003c48",  // MAFWMIDIFactoryBeBoB::systemWillSleep
            "0x00003532",  // MAFWMIDIFactoryBeBoB::setMIDIDevice:offline:
        };
        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : addrs) {
            Address addr = currentProgram.getAddressFactory().getAddress(a);
            Function f = fm.getFunctionAt(addr);
            if (f == null) { pw.println("=== " + a + " [no function] ==="); continue; }
            pw.println("=== " + a + "  " + f.getName(true) + "  size=" + f.getBody().getNumAddresses() + " ===");
            DecompileResults r = d.decompileFunction(f, 120, monitor);
            if (r == null || r.getDecompiledFunction() == null) { pw.println("[decomp failed]"); continue; }
            pw.println(r.getDecompiledFunction().getC());
            pw.println();
        }
        pw.close();
        println("Wrote " + out);
    }
}
