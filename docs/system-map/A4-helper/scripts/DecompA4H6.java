// H6 — Error handling + retry. Decompile the error / teardown / status paths.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H6 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h6-error-retry.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[][] targets = {
            {"0x000030eb", "-[MAFireWireLoader deviceMatched:]"},
            {"0x00002f6a", "-[MAFireWireLoader updateViewForDevice:]"},
            {"0x00003664", "-[MAFireWireLoader driverInstanceTerminated:]"},
            {"0x000035d5", "-[MAFireWireLoader driverInstanceMatched:]"},
            {"0x00002da7", "-[MAFireWireLoader unloadTheDriver]"},
            {"0x00002d6c", "-[MAFireWireLoader unloadDriverOnce]"},
            {"0x0000306d", "-[MAFireWireLoader quitView]"},
            {"0x00005884", "-[MAFireWireFirmwareFile firmwareValidationFailed:]"},
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
