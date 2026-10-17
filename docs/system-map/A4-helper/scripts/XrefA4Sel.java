// A4 — list code references to ObjC selector pointers / string symbols whose name contains $A4_PAT.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class XrefA4Sel extends GhidraScript {
    @Override
    public void run() throws Exception {
        String pat = System.getenv("A4_PAT");
        File log = new File(System.getenv("A4_OUT"), "decomp/a4-xref-" + pat + ".log");
        PrintWriter pw = new PrintWriter(new FileWriter(log));
        SymbolTable st = currentProgram.getSymbolTable();
        ReferenceManager rm = currentProgram.getReferenceManager();
        FunctionManager fm = currentProgram.getFunctionManager();
        SymbolIterator it = st.getAllSymbols(true);
        while (it.hasNext()) {
            Symbol s = it.next();
            if (!s.getName().contains(pat)) continue;
            pw.println("SYMBOL " + s.getName() + " @ " + s.getAddress());
            ReferenceIterator ri = rm.getReferencesTo(s.getAddress());
            while (ri.hasNext()) {
                Reference r = ri.next();
                Function f = fm.getFunctionContaining(r.getFromAddress());
                pw.println("   ref from " + r.getFromAddress() + " in " + (f == null ? "?" : f.getName()));
            }
        }
        pw.close();
        println("Wrote " + log);
    }
}
