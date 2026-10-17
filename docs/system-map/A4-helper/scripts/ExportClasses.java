// Export C++/Obj-C class hierarchy inferred from namespace structure.
// Writes to <project>/../ghidra-exports/classes-<programname>.md.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportClasses extends GhidraScript {
    @Override
    public void run() throws Exception {
        String progName = currentProgram.getName().replaceAll("[^A-Za-z0-9._-]", "_");
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        String out = new File(outDir, "classes-" + progName + ".md").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        pw.println("# Ghidra class listing — " + progName + "\n");

        SymbolTable st = currentProgram.getSymbolTable();
        Map<String, List<String>> byClass = new TreeMap<>();

        for (Symbol s : st.getAllSymbols(false)) {
            if (s.getSymbolType() != SymbolType.FUNCTION) continue;
            Namespace ns = s.getParentNamespace();
            if (ns == null || ns.isGlobal()) continue;
            String cls = ns.getName(true);
            byClass.computeIfAbsent(cls, k -> new ArrayList<>())
                   .add(String.format("- `0x%s` %s", s.getAddress(), s.getName(false)));
        }

        for (Map.Entry<String, List<String>> e : byClass.entrySet()) {
            pw.println("## " + e.getKey() + "  (" + e.getValue().size() + " methods)");
            pw.println();
            for (String m : e.getValue()) pw.println(m);
            pw.println();
        }
        pw.close();
        println("Wrote " + byClass.size() + " classes to " + out);
    }
}
