import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P9 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Polling model (CMiddleWareDevice + CDeviceSettings)
            "000639ea", // CMiddleWareDevice::AddMWPollingType
            "00063a72", // CMiddleWareDevice::RemoveMWPollingType
            "00063b16", // CMiddleWareDevice::SuspendAllPolling
            "00063b42", // CMiddleWareDevice::ResumeAllPolling
            "000663cc", // CDeviceSettings::AddPollingType
            "000663f0", // CDeviceSettings::RemovePollingType
            // Per-view polling
            "00053104", // CSetMgrSubView::StartViewPolling
            "00053114", // CSetMgrSubView::StopViewPolling
            // Inbound dispatchers (middleWareMessage*)
            "000d9f8c", // CMetersView::UpdateMixerPeakLevels
            "000d9ff0", // CMetersView::UpdateOutputPeakLevels
            "000de492", // CMixerView::UpdateOutputPeakLevels
            "000de4ec", // CMixerView::UpdateMixerPairPeakLevels
            "000e1d32", // COutputView::UpdateOutputPairPeakLevels
            "000e1dce", // COutputView::UpdateMixerPeakLevels
            // CXDevice level getters that feed meters
            "0006d85e", // CXDevice::GetChannelLevelProperty
            "00072138", // CXDevice::GetNumberOfDirectMonitorChannels
            "00072166", // CXDevice::GetNumberOfAuxMonitorSwitchChannels
            "00072180", // CXDevice::GetSWReturnPeaksAreEmbedded
            "00072e86", // CX1814Device::GetSWReturnPeaksAreEmbedded
            // 1814-specific MiniMetersView ties
            "0009e236", // CFW1814MiniMetersView::GetPortSettings
            "0009e2b0"  // CFW1814MiniMetersView::AdaptUIElementsToSettings
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
