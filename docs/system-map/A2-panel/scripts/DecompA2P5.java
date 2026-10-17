import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P5 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // CFW1814HardwareView — UI → settings mapping + Listen loop
            "00051630", // LoadUIElements
            "00051df8", // LoadSyncSourceBtns
            "00051f04", // ListenToUIElements
            "00051ff6", // ListenToSyncSourceBtns
            "00052040", // ListenToMessage (big: UI event dispatcher)
            "0005296c", // MapMOMButtonToSetting
            "00052988", // MapOpticalInputButtonToSetting
            "0005299e", // AdaptPortsToSettings
            "00052a6a", // MapOpticalOutputButtonToSetting
            "00052a80", // MapActiveInputButtonToSetting
            "00052a96", // MapMOMSettingToButton
            "00052ab8", // UpdateHPSourceButtonLabels
            "00052ce4", // MapOpticalInputSettingToButton
            "00052d06", // MapOpticalOutputSettingToButton
            "00052d28", // MapActiveInputSettingToButton
            "00052d4a", // ListenToMessage (small variant)
            "00052d74", // AdaptUIElementsToSettings
            "00053056", // MapClockSourceButtonToSetting
            "0005307c"  // MapClockSourceSettingToButton
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 120, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
