import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;

public class DecompQ14 extends GhidraScript {
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        String[] wants = {
            "m_audio_b_FWKIPCUserClient::initWithTask",
            "m_audio_b_FWKIPCUserClient::start",
            "m_audio_b_FWKIPCUserClient::stop",
            "m_audio_b_FWKIPCUserClient::registerNotificationPort",
            "m_audio_b_FWKIPCUserClient::SendMachMessage",
            "m_audio_b_FWKIPCUserClient::message",
            "m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage",
            "m_audio_b_FWKIPCUserClient::SerializeModifiedProperties",
            "m_audio_b_FWKIPCUserClient::AddModifiedProperties",
            "m_audio_b_FWKIPCUserClient::UserClientAdded",
            "m_audio_b_FWKIPCUserClient::UserClientRemoved",
            "m_audio_b_FWKIPCUserClient::clientClose"
        };
        for (String w : wants) {
            for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
                if (f.getName(true).equals(w)) {
                    DecompileResults r = di.decompileFunction(f, 60, monitor);
                    println("=== " + w + " @ " + f.getEntryPoint() + " ===");
                    println(r.getDecompiledFunction().getC());
                    break;
                }
            }
        }
    }
}
