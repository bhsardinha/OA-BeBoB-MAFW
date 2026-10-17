// A4 — helper preference/settings push: MAFireWireDeviceInstance methods + driverInstanceMatched/Terminated.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompA4Prefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = System.getenv("A4_OUT");
        File log = new File(outDir, "decomp/a4-prefs.log");
        log.getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(log));
        String[][] targets = {
            {"0x000023a0", "+[MAFireWireDeviceInstance deviceInstanceForService:]"},
            {"0x000026df", "-[MAFireWireDeviceInstance receivedPropertyChangeMessage:]"},
            {"0x00002829", "-[MAFireWireDeviceInstance defaultsForUser:]"},
            {"0x00002892", "-[MAFireWireDeviceInstance commitSettings:]"},
            {"0x0000290a", "-[MAFireWireDeviceInstance configureDCLLatency]"},
            {"0x00002afe", "-[MAFireWireDeviceInstance loadPrefsForUser:]"},
            {"0x00002c13", "-[MAFireWireDeviceInstance service:changedProperties:]"},
            {"0x000035d5", "-[MAFireWireLoader driverInstanceMatched:]"},
            {"0x00003664", "-[MAFireWireLoader driverInstanceTerminated:]"},
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
            pw.println(r != null && r.decompileCompleted() ? r.getDecompiledFunction().getC() : "[decompile failed]");
            pw.println();
        }
        pw.close();
        println("Wrote " + log);
    }
}
