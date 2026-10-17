// H1 — Entry + argv dispatch.
// Decompile _main plus the plausible argv-dispatch targets (MAFireWireCommunicator start/stop).
// Writes to $A4_OUT/decomp/a4-h1-entry-dispatch.log (fallback: hard path under A4-helper/).

import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H1 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h1-entry-dispatch.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[] targets = {
            "_main",
            "entry",
            "-[MAFireWireCommunicator init]",
            "-[MAFireWireCommunicator start]",
            "-[MAFireWireCommunicator stop]",
            "-[MAFireWireLoader init]",
            "-[MAFireWireLoader unloadTheDriver]",
            "-[MAFireWireLoader unloadDriverOnce]",
        };

        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();

        for (String name : targets) {
            Function hit = null;
            for (Function f : fm.getFunctions(true)) {
                String n = f.getName(true);
                if (n.equals(name) || n.endsWith(name) || n.replace(":", "_").equals(name.replace(":", "_"))) {
                    hit = f; break;
                }
            }
            if (hit == null) {
                pw.println("=== " + name + " ===");
                pw.println("[not found]\n");
                continue;
            }
            DecompileResults r = di.decompileFunction(hit, 60, monitor);
            pw.println("=== " + hit.getName(true) + " @ 0x" + hit.getEntryPoint() + " ===");
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
