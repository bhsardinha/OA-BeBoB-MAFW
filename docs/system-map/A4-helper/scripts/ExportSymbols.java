// Export the full function list with addresses, sizes, and demangled C++ names.
// Writes TSV to <project>/../ghidra-exports/functions-<programname>.tsv so multiple
// imports in one project don't collide.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;

public class ExportSymbols extends GhidraScript {
    @Override
    public void run() throws Exception {
        String progName = currentProgram.getName().replaceAll("[^A-Za-z0-9._-]", "_");
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        String out = new File(outDir, "functions-" + progName + ".tsv").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        pw.println("address\tsize\tname\tsignature\tcalls_count");
        FunctionManager fm = currentProgram.getFunctionManager();
        int n = 0;
        for (Function f : fm.getFunctions(true)) {
            long size = f.getBody().getNumAddresses();
            String name = f.getName(true);
            String sig  = f.getPrototypeString(false, false).replace('\t', ' ').replace('\n', ' ');
            int calls = 0;
            try { calls = f.getCalledFunctions(monitor).size(); } catch (Exception e) {}
            pw.printf("%s\t%d\t%s\t%s\t%d%n",
                f.getEntryPoint(), size, name, sig, calls);
            n++;
        }
        pw.close();
        println("Wrote " + n + " functions to " + out);
    }
}
