import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2Batch1 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // P1 — device discovery + user-client open path
            "0006c2b8", // CXDevice::CXDevice (ctor — opens IOKit connection)
            "0006cfd6", // CXDevice::RegisterForNotifications
            "0006c23e", // CXDevice::FetchDeviceForService
            "000648e0", // CDeviceList::MakeDeviceList
            "00064c28", // CDeviceList::InitDeviceList
            "00064d64", // CDeviceList::AddDevice
            "00063df2", // CMiddleWareManager::AddDevice (device form)
            "000635f2", // CMiddleWareDevice::CMiddleWareDevice
            // P2 — notification receive path
            "00070eda", // CXDevice::StaticHandleMachMessage (CFMachPort cb)
            "00070f18", // CXDevice::HandleNotification
            "000811aa", // ServiceDispatchCallback
            "0008108d", // -[MAServiceNotification initWithDictionary:port:type:target:selector:]
            // P3 — property wiring  (contains NSString key refs)
            "0006ca12", // CXDevice::InstallPropertyChangeHandlers
            "0006d660", // CXDevice::Get32BitNumberProperty
            "0006d7c6", // CXDevice::Set32BitNumberProperty
            "0006dafa", // CXDevice::HaveQueuedProperties
            "0006db2c", // CXDevice::SendQueuedProperties
            // Settings RPC primitives
            "0006378a", // CMiddleWareDevice::GetCPSetting
            "000637de", // CMiddleWareDevice::SetCPSetting
            "00063832", // CMiddleWareDevice::GetCPIndexedSetting
            "0006388a", // CMiddleWareDevice::SetCPIndexedSetting
            "0006c1e6"  // CXDevice::Receive32BitValueChange
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
