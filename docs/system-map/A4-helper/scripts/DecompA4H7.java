// H7 — launchd lifecycle.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H7 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h7-launchd-lifecycle.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[][] targets = {
            {"0x000021ee", "_main"},
            {"0x00003c48", "-[MAFireWireCommunicator init]"},
            {"0x00003c8a", "-[MAFireWireCommunicator start]"},
            {"0x00003cd8", "-[MAFireWireCommunicator stop]"},
            {"0x00003a58", "-[MACommunicator serve]"},
            {"0x00003ac7", "+[MACommunicator serve:forName:]"},
            {"0x00003b92", "-[MACommunicator runServer:]"},
            {"0x00003bd0", "-[MACommunicator stopServer]"},
            {"0x00003bdc", "+[MACommunicator getServer:]"},
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
