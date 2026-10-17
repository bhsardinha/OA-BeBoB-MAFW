import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

// Prints the first N 32-bit slots of the vtable at the address given as script arg 0 (hex), arg 1 = slot count.
public class PN2VtDump extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        Address base = currentProgram.getAddressFactory().getAddress(a[0]);
        int n = Integer.parseInt(a[1]);
        for (int i = 0; i < n; i++) {
            Address s = base.add(i * 4);
            long v = currentProgram.getMemory().getInt(s) & 0xffffffffL;
            Address t = currentProgram.getAddressFactory().getAddress(String.format("%08x", v));
            Function f = t == null ? null : currentProgram.getFunctionManager().getFunctionAt(t);
            println(String.format("VT[+0x%x] = %08x %s", i * 4, v, f == null ? "?" : f.getName(true)));
        }
    }
}
