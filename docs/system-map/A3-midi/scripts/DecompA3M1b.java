// A3-M1 followup — decode the DICE thunks + ancillary.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DecompA3M1b extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m1b.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        String[] addrs = {
            "0x00001db7",  // _MAFireWireMIDIDICE_Start
            "0x00001e35",  // _MAFireWireMIDIDICE_Stop
            "0x00001ead",  // _MAFireWireMIDIDICE_Send
        };
        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : addrs) {
            Address addr = currentProgram.getAddressFactory().getAddress(a);
            Function f = fm.getFunctionAt(addr);
            if (f == null) { pw.println("=== " + a + " [no function] ==="); continue; }
            pw.println("=== " + a + "  " + f.getName(true) + "  size=" + f.getBody().getNumAddresses() + " ===");
            DecompileResults r = d.decompileFunction(f, 90, monitor);
            if (r == null || r.getDecompiledFunction() == null) { pw.println("[decomp failed]"); continue; }
            pw.println(r.getDecompiledFunction().getC());
            pw.println();
        }
        pw.close();
        println("Wrote " + out);
    }
}
