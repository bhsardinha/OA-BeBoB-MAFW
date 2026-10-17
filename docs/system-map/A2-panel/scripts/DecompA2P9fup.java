import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceManager;

public class DecompA2P9fup extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        ReferenceManager rm = currentProgram.getReferenceManager();

        // First: xrefs on IOKit externs to find callers of map-memory / scalar-method
        String[] externs = {
            "000f4e48", // _IOConnectMapMemory
            "000f4e4e", // _IOConnectMethodScalarIScalarO
            "000f4e5a"  // _IOConnectUnmapMemory
        };
        for (String a : externs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            println("--- XREFS TO " + a + " (" + currentProgram.getSymbolTable().getPrimarySymbol(ad) + ") ---");
            for (Reference r : rm.getReferencesTo(ad)) {
                Function f = currentProgram.getFunctionManager().getFunctionContaining(r.getFromAddress());
                println("  " + r.getFromAddress() + "  " + (f == null ? "<outside func>" : f.getName(true)));
            }
        }

        String[] addrs = {
            // The periodical pump — pulls peak data
            "000721f2", // TTimerTask<CXDevice>::DoTask
            // CX1814Device ctor — see what polling types / cookies are wired
            "000e9d2c", // CX1814Device::CX1814Device
            "000e9d6c"  // CX1814Device::CX1814Device (second variant)
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 240, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
