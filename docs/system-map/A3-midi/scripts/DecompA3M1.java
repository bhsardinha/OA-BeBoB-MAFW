// A3-M1 — CoreMIDI factory + driver interface + glue bridge.
// Decompiles the chain: NewFW410MIDIDriver -> MIDIDriverQueryInterface/AddRef/Release
//   -> MIDIDriver(C ABI stubs) -> MAFWMIDIPluginGlueBeBoB (C++ glue)
//   -> MAFWMIDIFactoryBeBoB::initWithDriver: + ::dealloc

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;
import java.util.*;

public class DecompA3M1 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m1.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));

        String[] addrs = {
            "0x00001be6",  // _NewFW410MIDIDriver
            "0x000019f3",  // MIDIDriverQueryInterface
            "0x00001b7f",  // MIDIDriverAddRef
            "0x00001b8d",  // MIDIDriverRelease
            "0x000018b4",  // MIDIDriverFindDevices (C stub)
            "0x000018c7",  // MIDIDriverStart
            "0x000018da",  // MIDIDriverStop
            "0x000018ea",  // MIDIDriverConfigure
            "0x000018fd",  // MIDIDriverSend
            "0x00001910",  // MIDIDriverEnableSource
            "0x00001923",  // MIDIDriverFlush
            "0x00001936",  // MIDIDriverMonitor
            "0x0000194a",  // MIDIDriver::MIDIDriver (base ctor)
            "0x00001984",  // MIDIDriver::~MIDIDriver (D0)
            "0x00001bae",  // MIDIDriver::Start (default no-op)
            "0x00001bb6",  // MIDIDriver::Stop
            "0x00001bbe",  // MIDIDriver::Configure
            "0x00001bc6",  // MIDIDriver::Send
            "0x00001bce",  // MIDIDriver::EnableSource
            "0x00001bd6",  // MIDIDriver::Flush
            "0x00001bde",  // MIDIDriver::Monitor
            "0x00001ca6",  // MAFWMIDIPluginGlueBeBoB::C1 (ctor thunk)
            "0x00001cb0",  // MAFWMIDIPluginGlueBeBoB::C2 (base ctor)
            "0x00001d54",  // ::~D0
            "0x00001d70",  // ::~D1
            "0x00001d7a",  // ::~D2
            "0x00001d84",  // ::FindDevices
            "0x00001d8c",  // ::Start
            "0x00001d9a",  // ::Stop
            "0x00001da4",  // ::Send
            "0x0000337b",  // MAFWMIDIFactoryBeBoB::initWithDriver:
            "0x000034d3",  // MAFWMIDIFactoryBeBoB::dealloc
        };

        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();

        for (String a : addrs) {
            Address addr = currentProgram.getAddressFactory().getAddress(a);
            Function f = fm.getFunctionAt(addr);
            if (f == null) { pw.println("=== " + a + " [no function] ==="); continue; }
            pw.println("=== " + a + "  " + f.getName(true) + "  size=" + f.getBody().getNumAddresses() + " ===");
            DecompileResults r = d.decompileFunction(f, 90, monitor);
            if (r == null || r.getDecompiledFunction() == null) { pw.println("[decomp failed]"); continue; }
            pw.println(r.getDecompiledFunction().getC());
            pw.println();
        }
        pw.close();
        println("Wrote " + out);
    }
}
