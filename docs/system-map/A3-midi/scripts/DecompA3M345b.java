import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.Memory;
import java.io.*;

public class DecompA3M345b extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m345b.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));

        // Resolve DAT_00007470 (timer interval double constant)
        Memory mem = currentProgram.getMemory();
        Address intervalAddr = currentProgram.getAddressFactory().getAddress("0x00007470");
        long rawBits = mem.getLong(intervalAddr);
        double interval = Double.longBitsToDouble(rawBits);
        pw.println("=== DAT_00007470 (CFRunLoopTimer interval) ===");
        pw.println("  raw: 0x" + Long.toHexString(rawBits) + "  double: " + interval + " seconds");
        pw.println();

        String[] addrs = {
            "0x00002728",  // rebuildEndpointArray
            "0x00002ee8",  // HandleMIDIInputCallout
            "0x000020fe",  // StaticHandleMachMessage
            "0x00002f00",  // stop
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
