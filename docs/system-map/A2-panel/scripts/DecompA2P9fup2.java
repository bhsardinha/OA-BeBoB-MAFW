import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.mem.Memory;

public class DecompA2P9fup2 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);

        // Dump CXDevice::CXDevice (map-memory caller) and CXDevice::~CXDevice
        String[] addrs = {
            "0006c54a", // CXDevice::CXDevice (ctor)
            "0006d148", // CXDevice::TerminationWaitThread (uses ScalarIScalarO)
            "0006d256", // CXDevice::~CXDevice
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionContaining(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 240, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }

        // Dump the CX1814Device LPeriodical-subobject vtable at 001cce20 (10 slots)
        Memory mem = currentProgram.getMemory();
        Address vt = currentProgram.getAddressFactory().getAddress("001cce20");
        println("=== CX1814Device LPeriodical-subobject vtable @ 001cce20 ===");
        for (int i = 0; i < 20; i++) {
            Address slot = vt.add(i * 4);
            int ptr = mem.getInt(slot) & 0xffffffff;
            String tgt = "0x" + Integer.toHexString(ptr);
            if (ptr != 0) {
                Address targetAddr = currentProgram.getAddressFactory().getAddress(Integer.toHexString(ptr));
                Function tf = currentProgram.getFunctionManager().getFunctionAt(targetAddr);
                if (tf != null) tgt += "  " + tf.getName(true);
            }
            println("  [+0x" + Integer.toHexString(i*4) + "] = " + tgt);
        }

        // Also dump CX1814Device main vtable at 001ccd98
        Address vt2 = currentProgram.getAddressFactory().getAddress("001ccd98");
        println("=== CX1814Device main vtable @ 001ccd98 (first 40 slots) ===");
        for (int i = 0; i < 40; i++) {
            Address slot = vt2.add(i * 4);
            int ptr = mem.getInt(slot) & 0xffffffff;
            String tgt = "0x" + Integer.toHexString(ptr);
            if (ptr != 0) {
                Address targetAddr = currentProgram.getAddressFactory().getAddress(Integer.toHexString(ptr));
                Function tf = currentProgram.getFunctionManager().getFunctionAt(targetAddr);
                if (tf != null) tgt += "  " + tf.getName(true);
            }
            println("  [+0x" + Integer.toHexString(i*4) + "] = " + tgt);
        }
    }
}
