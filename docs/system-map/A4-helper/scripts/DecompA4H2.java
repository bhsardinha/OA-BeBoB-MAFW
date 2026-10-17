// H2 — Decompile by hard-coded address (reliable; name matching was flaky in Ghidra 12 on
// Obj-C i386 symbols with embedded brackets).

import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h2-bootloader-detection.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        // addr, label
        String[][] targets = {
            {"0x00003c48", "-[MAFireWireCommunicator init]"},
            {"0x00003c8a", "-[MAFireWireCommunicator start]"},
            {"0x00003cd8", "-[MAFireWireCommunicator stop]"},
            {"0x00002c3d", "-[MAFireWireLoader init]"},
            {"0x000030eb", "-[MAFireWireLoader deviceMatched:]"},
            {"0x000035d5", "-[MAFireWireLoader driverInstanceMatched:]"},
            {"0x00003664", "-[MAFireWireLoader driverInstanceTerminated:]"},
            {"0x00003541", "-[MAFireWireLoader deviceForService:]"},
            {"0x00002f6a", "-[MAFireWireLoader updateViewForDevice:]"},
        };

        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();

        for (String[] t : targets) {
            Address addr = currentProgram.getAddressFactory().getAddress(t[0]);
            Function f = fm.getFunctionAt(addr);
            pw.println("=== " + t[1] + " @ " + t[0] + " ===");
            if (f == null) { pw.println("[no function at addr]\n"); continue; }
            DecompileResults r = di.decompileFunction(f, 90, monitor);
            if (r != null && r.decompileCompleted()) {
                pw.println(r.getDecompiledFunction().getC());
            } else {
                pw.println("[decompile failed: " + (r == null ? "null" : r.getErrorMessage()) + "]");
            }
            pw.println();
        }
        pw.close();
        println("Wrote decomp to " + log);
    }
}
