// Resolve CFString constants at given addresses.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.data.*;
import java.io.*;

public class DumpStrings extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] addrs = {
            "0x0000ae28", "0x0000af48", "0x0000af68",
            "0x0000af88", "0x0000afa8"
        };
        Memory mem = currentProgram.getMemory();
        for (String a : addrs) {
            Address addr = currentProgram.getAddressFactory().getAddress(a);
            println("=== " + a + " ===");
            // CFString struct: isa(8), flags(8), ptr(8), len(8)
            try {
                long ptr = mem.getLong(addr.add(16));
                Address strAddr = currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(ptr);
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < 128; i++) {
                    byte b = mem.getByte(strAddr.add(i));
                    if (b == 0) break;
                    sb.append((char)(b & 0xff));
                }
                println("  -> \"" + sb + "\"");
            } catch (Exception e) {
                // maybe it IS the string, try direct
                try {
                    StringBuilder sb = new StringBuilder();
                    for (int i = 0; i < 64; i++) {
                        byte b = mem.getByte(addr.add(i));
                        if (b == 0) break;
                        if (b >= 0x20 && b < 0x7f) sb.append((char)b);
                        else sb.append('.');
                    }
                    println("  raw: \"" + sb + "\"");
                } catch (Exception e2) {
                    println("  err: " + e2.getMessage());
                }
            }
        }
    }
}
