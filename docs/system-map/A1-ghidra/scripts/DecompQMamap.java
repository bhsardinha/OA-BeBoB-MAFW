// Q-MAMAP — decompile the FWSettings* classes that register and write every MA* key. Addresses in env QM_ADDRS (comma separated hex).
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompQMamap extends GhidraScript {
    @Override
    public void run() throws Exception {
        String out = System.getenv("QM_OUT");
        String[] addrs = System.getenv("QM_ADDRS").split(",");
        PrintWriter pw = new PrintWriter(new FileWriter(new File(out)));
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a.trim());
            Function f = fm.getFunctionAt(ad);
            if (f == null) { pw.println("=== MISS " + a + " ===\n"); continue; }
            pw.println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            DecompileResults r = di.decompileFunction(f, 120, monitor);
            pw.println(r != null && r.decompileCompleted() ? r.getDecompiledFunction().getC() : "[decompile failed]");
            pw.println();
        }
        pw.close();
        println("Wrote " + out);
    }
}
