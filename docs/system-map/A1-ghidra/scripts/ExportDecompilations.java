// Decompile a curated list of key methods (from Q1-Q12) and dump the pseudo-C
// to one file per method under ghidra-exports/decomp/.

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportDecompilations extends GhidraScript {
    // Match on substrings — Ghidra will mangle slightly differently per demangler run.
    private static final String[] TARGETS = {
        // Q1 — DCL input / builder
        "FWDCLInputProgram::Initialize",
        "FWDCLInputProgram::ReconnectAfterBusReset",
        "FWDCLInputProgram::AddTimestampedTransferPackets",
        "FWDCLBuilder::AddTransferPackets",
        "FWDCLBuilder::ModifyJumpLabel",
        "FWDCLBuilder::CheckDCLOverrun",
        "InputDCLCallback",
        "InputDCLStaticCallback",
        "InputDCLStaticOverrunCallback",
        "InputDCLCallbackOverrun",
        // Q1b
        "FWDCLProgram::CreateIsochChannelAndPorts",
        "FWDCLProgram::Disassemble",
        "FWDCLOutputProgram::Connect",
        "FWDCLInputProgram::Connect",
        // Q2
        "FWDCLProgram::SetupCIP",
        "FWDCLProgram::CalcIsochPacketHeaders",
        // Q3
        "FW1814Device::SetupAudioEngine",
        "FWAudiophileDevice::SetupAudioEngine",
        "FWEngine2::ConnectAllPlugs",
        "FWEngine2::ConnectAudioMIDIChannels",
        "FWEngine2::initHardware",
        // Q4
        "FW1814Device::ReconnectP2P",
        "FW1814Device::BusReset",
        "FWBaseEngine::ReconnectAfterBusReset",
        // Q5
        "FW1814Device::SetClockSourceInternal",
        "FW1814Device::SetBlankSlateClockSource",
        "FWAudiophileDevice::SetClockSourceInternal",
        // Q6
        "FWHSCI::Pause",
        "FWHSCI::Resume",
        "FWHSCI::HSCITimerFired",
        "FWHSCI::HSCIReadComplete",
        "FWHSCI::FWHSCIDeviceCallback",
        "FWHSCI::HSCISafelyComplete",
        "FWHSCI::Start",
        "FWHSCI::Stop",
        // Q7
        "FWMetaNub::StateString",
        "FWMetaNub::message",
        "FWMetaNub::stop",
        "FWMetaNub::ReregisterDeviceAfterFirmwareReset",
        "FWMetaNub::ReregisterDeviceTimerFired",
        "FWMetaNub::RegregisterDeviceThread",
        "FWMetaNub::FireWireWriteCommand",
        "FWMetaNub::FireWireReadCommand",
        "FWMetaNub::FirmwareWriteCommand",
        "FWMetaNub::FirmwareStart",
        "FWMetaNub::FirmwareReset",
        "FWMetaNub::FirmwareHalt",
        "FWMetaNub::DownloadStop",
        "FWMetaNub::DownloadSendNextBlock",
        "FWMetaNub::DownloadSectionStart",
        "FWMetaNub::FirmwareUpdateBeBoB32",
        "FWMetaNub::FirmwareUpdateBeBoB251",
        "FWMetaNub::FirmwareCacheInfoRegisters",
        // Q8
        "MakeAppleFWAudioGoAway::probe",
        // Q9
        "FW1814Device::WorkaroundFor1814InputDelay",
        "FW1814Device::ResetRearrangementProcs",
        // Q10
        "FW1814Device::GetRoundTripLatencyForFDF",
        "FWAudiophileDevice::GetRoundTripLatencyForFDF",
        // Q11
        "FWUserClient::getTargetAndMethodForIndex",
        "FWUserClient::EntryStartMIDI",
        "FWUserClient::EntryStopMIDI",
        "FWUserClient::EntrySendMIDI",
        "FWUserClient::EntryDiceSetRequest",
        "FWUserClient::EntryDiceGetRequest",
        "FWUserClient::ActionStartMIDI",
        "FWUserClient::ActionStopMIDI",
        "FWUserClient::ActionSendMIDI",
        "FWUserClient::CoreStartMIDI",
        "FWUserClient::CoreStopMIDI",
        "FWUserClient::clientMemoryForType",
        "FWUserClient::clientClose",
        // Q12
        "FWEngine2::ActivateAC3Output",
        "FWEngine2::DeactivateAC3Output",
        "FWEngine2::SetAC3OutputState",
        "FWEngine2::ClipAC3Output",
        "FWEngine2::StartAC3Output",
        "FWBaseEngine::AddAC3Format",
        "FWBaseEngine::EnableAC3Format",
        "FWBaseEngine::DisableAC3Format",
        // Lifecycle anchors we have not yet hand-decoded in A1
        "FW1814Device::start",
        "FW1814Device::stop",
        "FW1814Device::init",
        "FW1814Nub::probe",
        "FW1814Nub::start",
        "FW1814Nub::matchPropertyTable",
        "FW1814Bootloader::probe",
        "FWEngine2::performAudioEngineStart",
        "FWEngine2::performAudioEngineStop",
        "FWEngine2::performFormatChange",
        "FWEngine2::clipOutputSamples",
        "FWEngine2::convertInputSamples",
        "FWEngine2::StartAudio",
        "FWEngine2::StopAudio",
        "FWEngine2::DisconnectAllPlugs",
        "FWEngine2::ConvertSampleRate",
        "FWAVCAudioDevice::AVCVendorCommand",
        "FWAVCAudioDevice::AVCSelectorCommand",
        "FWAVCAudioDevice::IsAC3Supported",
    };

    @Override
    public void run() throws Exception {
        String outDir = "/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A1-ghidra/ghidra-exports/decomp";
        new File(outDir).mkdirs();

        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        di.setSimplificationStyle("decompile");

        FunctionManager fm = currentProgram.getFunctionManager();
        List<Function> all = new ArrayList<>();
        for (Function f : fm.getFunctions(true)) all.add(f);

        PrintWriter idx = new PrintWriter(new FileWriter(outDir + "/_INDEX.md"));
        idx.println("# Decompiled method index\n");

        int hits = 0, miss = 0;
        for (String want : TARGETS) {
            Function match = null;
            for (Function f : all) {
                String n = f.getName(true);
                if (n.contains(want)) { match = f; break; }
            }
            if (match == null) {
                idx.printf("- **MISS** `%s`%n", want);
                miss++;
                continue;
            }
            DecompileResults r = di.decompileFunction(match, 60, monitor);
            if (r == null || !r.decompileCompleted()) {
                idx.printf("- **FAIL** `%s` @ `%s`%n", want, match.getEntryPoint());
                miss++;
                continue;
            }
            String safe = want.replace("::", "__").replaceAll("[^A-Za-z0-9_]", "_");
            String path = outDir + "/" + safe + ".c";
            PrintWriter pw = new PrintWriter(new FileWriter(path));
            pw.println("// " + match.getName(true));
            pw.println("// Entry: " + match.getEntryPoint() + "  Size: "
                       + match.getBody().getNumAddresses() + " bytes");
            pw.println("// Prototype: " + match.getPrototypeString(false, false));
            pw.println();
            pw.println(r.getDecompiledFunction().getC());
            pw.close();
            idx.printf("- [`%s`](%s) @ `0x%s`%n", want, safe + ".c", match.getEntryPoint());
            hits++;
        }
        idx.printf("%n**%d hit, %d miss.**%n", hits, miss);
        idx.close();
        println("Decompiled " + hits + " / " + (hits + miss) + " targets.");
    }
}
