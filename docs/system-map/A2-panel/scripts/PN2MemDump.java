import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

// Hex-dumps arg 1 bytes at the first occurrence of the ASCII string in arg 0, or at the address given as "@hex".
public class PN2MemDump extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        // arg 0: ASCII string to find, or "@hex" for a direct address.
        Address hit;
        if (a[0].startsWith("@")) hit = currentProgram.getAddressFactory().getAddress(a[0].substring(1));
        else hit = currentProgram.getMemory().findBytes(currentProgram.getMinAddress(), a[0].getBytes("US-ASCII"), null, true, monitor);
        if (hit == null) { println("NOTFOUND"); return; }
        int n = Integer.parseInt(a[1]);
        StringBuilder sb = new StringBuilder("DUMP @" + hit + "\n");
        for (int i = 0; i < n; i++) {
            if (i % 16 == 0) sb.append(String.format("\n+%02x: ", i));
            sb.append(String.format("%02x ", currentProgram.getMemory().getByte(hit.add(i)) & 0xff));
        }
        println(sb.toString());
    }
}
