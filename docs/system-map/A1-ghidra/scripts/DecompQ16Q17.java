import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompQ16Q17 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Q17 TPacketPool
            "0000a3e6", // Initialize
            "0000a564", // ctor
            "0000a4e8", // ForceReleaseMemory
            "0000a540", // Invalidate
            "0000a5bc", // GetIndexedPacket
            "0000a5e6", // ZeroElements
            "0000a738", // MarkGroupReadForInput
            "0000a784", // GetIndexedActivePacket
            "0000a7aa", // GetPacketFrameContainingSampleIndex
            "0000a83e", // RawPacketIndexFromSampleFrame
            "0000a86c", // SyncCurrentInputPacketIndex
            "0000a8b0", // InitGroupCIPsForOutput
            // Q16 FWRev1 vs FWRev2
            "000136aa", // Rev1 AllocateAudioEngine
            "000136c8", // Rev1 stop
            "000136da", // Rev1 GetRoundTripLatencyForFDF
            "00013734", // Rev1 AVCControlPlugSignalFormat
            "00012994", // Rev2 IsReceptionOfControlPacketsEnabled
            "00012a3e", // Rev2 initHardware
            "00012a54", // Rev2 GetRoundTripLatencyForFDF
            "00012aae", // Rev2 UpdatePeakMeters
            "00012ae8", // Rev2 GetAddressAndPlugForSyncType
            "00012b5c", // Rev2 performPowerStateChange
            "00012b9c", // Rev2 AllocateAudioEngine
            "00012c60", // Rev2 AVCSetupSyncStream
            "00012e7e", // Rev2 AVCControlPlugSignalFormat
            "00013078", // Rev2 stop
            "000130c8", // Rev2 StopReceptionOfControlPackets
            "000130f4", // Rev2 ResumeReceptionOfControlPackets
            "0001310c", // Rev2 PauseReceptionOfControlPackets
            "00013138"  // Rev2 StartReceptionOfControlPackets
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 90, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
