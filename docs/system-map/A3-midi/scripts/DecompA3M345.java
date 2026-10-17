// A3-M3/M4/M5 — async notifications, wire packet, TX+RX paths.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DecompA3M345 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String projDir = currentProgram.getDomainFile().getProjectLocator().getLocation();
        String out = new File(new File(projDir).getParentFile(), "ghidra-exports/decomp/a3-m345.log").getPath();
        new File(out).getParentFile().mkdirs();
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        String[] addrs = {
            "0x00002d47",  // MAFWMIDIDeviceBeBoB::start
            "0x00002682",  // MAFWMIDIDeviceBeBoB::dealloc
            "0x00002f7d",  // MAFWMIDIDeviceBeBoB::send:viaPort:
            "0x00002b1b",  // MAFWMIDIDeviceBeBoB::checkForMIDIInput
            "0x00001f37",  // MAFWBeBoBAsyncNotification::initWithType:connection:runLoop:target:selector:
            "0x000021d4",  // userClientNotificationOfType:connection:runLoop:target:selector:
            "0x0000228e",  // AsyncNotification::dealloc
            "0x00004b9c",  // TMIDIParser::ParseMIDIByte
            "0x00004d20",  // TMIDIParser::ReceiveMIDIByte
            "0x00004d70",  // TUserMIDIPort::Init
            "0x00004de8",  // TMIDIPort::CheckForNewBytes
            "0x00004e8e",  // TMIDIPort::CheckForNewRealtimePacket
            "0x00004ed4",  // TMIDIPort::CheckForNewTimestampedPacket
        };
        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : addrs) {
            Address addr = currentProgram.getAddressFactory().getAddress(a);
            Function f = fm.getFunctionAt(addr);
            if (f == null) { pw.println("=== " + a + " [no function] ==="); continue; }
            pw.println("=== " + a + "  " + f.getName(true) + "  size=" + f.getBody().getNumAddresses() + " ===");
            DecompileResults r = d.decompileFunction(f, 120, monitor);
            if (r == null || r.getDecompiledFunction() == null) { pw.println("[decomp failed]"); continue; }
            pw.println(r.getDecompiledFunction().getC());
            pw.println();
        }
        pw.close();
        println("Wrote " + out);
    }
}
