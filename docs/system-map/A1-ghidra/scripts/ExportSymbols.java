// Export the full function list with addresses, sizes, and demangled C++ names.
// Writes TSV to <project>/../../ghidra-exports/functions.tsv

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;

public class ExportSymbols extends GhidraScript {
    @Override
    public void run() throws Exception {
        // Resolve path relative to the Ghidra project location so A1/A2/etc. don't collide.
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/functions.tsv").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        pw.println("address\tsize\tname\tsignature\tcall_depth\tcalls_count");
        FunctionManager fm = currentProgram.getFunctionManager();
        int n = 0;
        for (Function f : fm.getFunctions(true)) {
            long size = f.getBody().getNumAddresses();
            String name = f.getName(true);
            String sig  = f.getPrototypeString(false, false).replace('\t', ' ').replace('\n', ' ');
            int depth = 0;
            try { depth = f.getCalledFunctions(monitor).size(); } catch (Exception e) {}
            pw.printf("%s\t%d\t%s\t%s\t%d\t%d%n",
                f.getEntryPoint(), size, name, sig, depth, f.getCalledFunctions(monitor).size());
            n++;
        }
        pw.close();
        println("Wrote " + n + " functions to " + out);
    }
}
