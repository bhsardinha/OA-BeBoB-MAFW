import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P8 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Rate + buffer-size UI load path (CHardwareView base)
            "000d69de", // LoadSampleRateControl
            "000d6950", // LoadSampleRateDisplay
            "000d6bb8", // LoadBufferSizeControl
            // The 968-byte cookie-switch for the hardware tab
            "000d7108", // ListenToMessage (base)
            // Adapt helpers
            "000d755c", // AdaptSampleRateDisplay
            "000d75f2", // AdaptSampleRateControl
            "000d6902", // LoadUIElements
            // Supporting rate/buffer table on CDeviceSettings
            "000664b0", // LoadSupportedSampleRates
            "000665a2", // GetSampleRateByIndex
            "000665d8", // GetSampleRateIndex
            "00066610", // LoadSupportedBufferSizes
            "00066624", // GetBufferSizeByIndex
            // Deferred rate push (seen in P7-fup)
            "0006c8c8", // CXDevice::SendNewSampleRateTimer
            // Controller-knob assignment (actual FW1814 "Global Settings" tab contents)
            "00084694", // LoadControllerAssignBtns
            "000848fa", // ListenToControllerAssignBtns
            "0008462c", // MapAssignSettingToButton
            "0008465c"  // MapAssignButtonToSetting
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
