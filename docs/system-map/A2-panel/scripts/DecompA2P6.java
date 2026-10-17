import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;

public class DecompA2P6 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] addrs = {
            // Per-channel-pair strip (level/pan/mute/solo + aux-send are on this view)
            "00055414", // CFW1814MixerChannelPairView::LoadUIElements
            "00055520", // CFW1814MixerChannelPairView::ListenToUIElements
            "0005556a", // CFW1814MixerChannelPairView::ListenToMessage       (big)
            "000556a8", // CFW1814MixerChannelPairView::MapMessageToHWPairID
            "000556e8", // CFW1814MixerChannelPairView::AdaptUIElementsToSettings
            // Mixer-level: port-driven channel materialization + rate-adapt + aux routing
            "00056a7e", // CFW1814MixerView::GetPortSettings
            "00056af8", // CFW1814MixerView::MapSettingsToChannelViewID
            "00056ce6", // CFW1814MixerView::ListenToMessage                   (338 B)
            "00056e38", // CFW1814MixerView::AdaptChannelsToPortSettings
            "00056f04", // CFW1814MixerView::AdaptAdatChannelsToSMUX
            "00057012", // CFW1814MixerView::AdaptToClockRate
            "000570b4"  // CFW1814MixerView::AdaptUIElementsToSettings
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
