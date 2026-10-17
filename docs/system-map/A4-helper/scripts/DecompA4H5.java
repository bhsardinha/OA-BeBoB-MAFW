// H5 — Status-UI IPC. Switch target: FirmwareStatusWindow.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4H5 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        File log = new File(outDir, "decomp/a4-h5-status-ui-ipc.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));

        String[][] targets = {
            {"0x00002372", "_main"},
            {"0x00002d28", "-[FirmwareAppDelegate applicationDidFinishLaunching:]"},
            {"0x000029e9", "-[FirmwareAppDelegate showWindowWithDeviceIcon:name:GUID:]"},
            {"0x00003173", "-[FirmwareAppDelegate(Messaging) checkForServer]"},
            {"0x0000322d", "-[FirmwareAppDelegate(Messaging) startServer]"},
            {"0x00002f67", "-[FirmwareAppDelegate setProgressIndicatorCurrent:maximum:]"},
            {"0x00003083", "-[FirmwareAppDelegate applicationWillTerminate:]"},
            {"0x00002cfd", "-[FirmwareAppDelegate terminate]"},
            {"0x000028fa", "-[FirmwareAppDelegate deviceIconWithName:]"},
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
