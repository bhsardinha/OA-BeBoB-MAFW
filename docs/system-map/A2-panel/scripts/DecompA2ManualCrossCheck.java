import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2ManualCrossCheck extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Item #2 — ADAT S/MUX gating polarity
            "0005299e", // CFW1814HardwareView::AdaptPortsToSettings
            // Item #5 — rate gating in mixer page
            "00057012", // CFW1814MixerView::AdaptToClockRate
            "0005502e", // CFW1814HPOutChannelPairView::AdaptToClockRate
            "000590bc", // CFW1814OutputView::AdaptToClockRate
            // Items #4, #8, #9 — HPAB dialog + its setting path
            "00053124", // ShowHPABSourceDialog
            "00053840", // FinishCreateSelf (1511 bytes — core of the dialog)
            "00053f52", // ListenToMessage (731 bytes — click dispatch)
            "0005486c", // SaveSettings (writes back to CDeviceSettings)
            "00054746", // LoadSettings
            "00053730", // SavePrefs (writes to NSUserDefaults-equivalent)
            "00053e28", // RestorePrefs
            "0005422e", // MapHPSourceButtonToSetting
            "00054476", // MapHPSourceSettingToButton
            "00054256"  // UpdateHPSourceButtons (completes #4 label mapping)
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 300, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
