// H4 — Firmware file selection + validation + CRC.

import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H4 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h4-firmware-selection.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[][] targets = {
            {"0x00005a90", "+[MAFireWireFirmwareFile firmwareFilesAtPath:forDevice:]"},
            {"0x00005734", "+[MAFireWireFirmwareFile firmwareWithPath:]"},
            {"0x00005cc8", "-[MAFireWireFirmwareFile validateFirmwareUpdate:]"},
            {"0x00005c7d", "-[MAFireWireFirmwareFile embeddedCRC]"},
            {"0x00005c8b", "-[MAFireWireFirmwareFile actualCRC]"},
            {"0x000055fc", "CalendarDateFromFirmwareDate"},
            {"0x00002542", "CanonicalGUIDForDevice"},
            {"0x00005884", "-[MAFireWireFirmwareFile firmwareValidationFailed:]"},
            {"0x0000589a", "-[MAFireWireFirmwareFile reportCorruptFirmwareToDelegate:format:]"},
            {"0x00005f15", "-[MAFireWireFirmwareFile date]"},
            {"0x00005801", "-[MAFireWireFirmwareFile dealloc]"},
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
