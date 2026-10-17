import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P7 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Base-class dispatchers (vtable entries P5/P6 call through)
            "00065970", // CDeviceSettings::GetSetting           (vtable+0xc)
            "000658a2", // CDeviceSettings::UpdateSettingAndHW   (vtable+0x18)
            "000657f0", // CDeviceSettings::UpdateSetting        (cache-only)
            "00065750", // CDeviceSettings::ListenToMessage      (notification bridge)
            // FW1814 subclass: the cookie switch
            "00059836", // CFW1814DeviceSettings::SetSettingData (2093 B)
            "0005a19c", // CFW1814DeviceSettings::GetSettingData (1478 B)
            // Hardware sync (seeds cookie table)
            "0005b5f8", // CFW1814DeviceSettings::ReadSettingsFromHardware
            "0005bf10"  // CFW1814DeviceSettings::WriteSettingsToHardware
        };
        for (String a : addrs) {
            Address ad = currentProgram.getAddressFactory().getAddress(a);
            Function f = currentProgram.getFunctionManager().getFunctionAt(ad);
            if (f == null) { println("MISS " + a); continue; }
            DecompileResults r = di.decompileFunction(f, 180, monitor);
            println("=== " + f.getName(true) + " @ " + f.getEntryPoint() + " ===");
            println(r.getDecompiledFunction().getC());
        }
    }
}
