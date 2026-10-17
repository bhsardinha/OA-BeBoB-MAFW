// H9 — bcd parsing in helper. Remaining methods not covered by H4.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H9 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h9-bcd-reader.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[][] targets = {
            {"0x00005734", "+[MAFireWireFirmwareFile firmwareWithPath:]"},
            {"0x00005801", "-[MAFireWireFirmwareFile dealloc]"},
            {"0x00005c7d", "-[MAFireWireFirmwareFile embeddedCRC]"},
            {"0x00005c8b", "-[MAFireWireFirmwareFile actualCRC]"},
            {"0x00005f0a", "-[MAFireWireFirmwareFile data]"},
            {"0x00005f15", "-[MAFireWireFirmwareFile date]"},
            {"0x00005f48", "-[MAFireWireFirmwareFile name]"},
            {"0x0000589a", "-[MAFireWireFirmwareFile reportCorruptFirmwareToDelegate:format:]"},
            {"0x00005917", "+[MAFireWireFirmwareFile automaticUpdatesInhibited]"},
            {"0x00005956", "+[MAFireWireFirmwareFile setInhibitAutomaticUpdates:]"},
            {"0x000059f2", "+[MAFireWireFirmwareFile forceNextAutomaticUpdate]"},
            {"0x00005a31", "+[MAFireWireFirmwareFile automaticUpdatedForced]"},
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
