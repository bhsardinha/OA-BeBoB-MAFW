// A3-M2 — device discovery + user-client open.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DecompA3M2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m2.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        String[] addrs = {
            "0x000038b6",  // driverInstanceMatched:
            "0x00003bc8",  // driverInstanceTerminated:
            "0x000035ff",  // fetchMIDIDeviceForService:
            "0x00003743",  // createDeviceProperties:withService:
            "0x00002363",  // MAFWMIDIDeviceBeBoB::instantiateDriver:withService:withMIDIDevice:
            "0x00004111",  // openConnection
            "0x00004140",  // closeConnection
            "0x0000407f",  // initWithService:
            "0x000040d1",  // deviceWithService:
            "0x0000402b",  // deviceFromIOKitDevice:connection:
            "0x000041ea",  // IOKitService::dealloc
            "0x00003532",  // setMIDIDevice:offline:
            "0x00003579",  // setAllDevicesToOffline
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
