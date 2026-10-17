// Dump all defined strings (cstring + CFString) with address + content.
// Writes to <project>/../ghidra-exports/strings-<programname>.txt.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.data.*;
import ghidra.program.util.*;
import java.io.*;
import java.util.*;

public class DumpStrings extends GhidraScript {
    @Override
    public void run() throws Exception {
        String progName = currentProgram.getName().replaceAll("[^A-Za-z0-9._-]", "_");
        String outDir = System.getenv("A4_OUT");
        if (outDir == null) outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A4-helper/ghidra-exports";
        String out = new File(outDir, "strings-" + progName + ".txt").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));

        Listing listing = currentProgram.getListing();
        DataIterator it = listing.getDefinedData(true);
        int n = 0;
        while (it.hasNext()) {
            Data d = it.next();
            DataType dt = d.getDataType();
            String tn = dt.getName().toLowerCase();
            if (tn.contains("string") || tn.contains("char")) {
                Object v = d.getValue();
                if (v == null) continue;
                String s = v.toString().replace("\n", "\\n").replace("\r", "\\r");
                if (s.length() < 2) continue;
                pw.printf("0x%s\t%s\t%s%n", d.getAddress(), dt.getName(), s);
                n++;
            }
        }
        pw.close();
        println("Wrote " + n + " strings to " + out);
    }
}
