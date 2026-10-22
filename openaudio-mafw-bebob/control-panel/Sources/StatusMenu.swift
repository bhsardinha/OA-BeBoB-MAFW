// Plan v8 AP-5.5 (user, 2026-10-21; a clone feature: the reference has a Carbon panel and a prefpane, no status item) -- menu bar mode.
// A status icon shows the selected unit (online / muted / no sync; dimmed with no unit); a click opens the panel, a right click the menu, which
// acts on that unit: open panel, volume sliders (outputs console law, phones monitor law), mute, dim, presets, the big meter, Audio MIDI Setup, firmware
// check, sync, sample rate, panel presentation, rate and lock status (order: user, 2026-10-21). The Dock / menu bar choice and launch
// at login are per-Mac panel preferences (PanelPrefs); AppDelegate applies them (activation policy, status item).
import AppKit

enum PanelPrefs {
    private static let d = UserDefaults.standard
    static var showInDock: Bool { get { return d.object(forKey: "ShowInDock") as? Bool ?? true } set { d.set(newValue, forKey: "ShowInDock") } }
    static var showInMenuBar: Bool { get { return d.object(forKey: "ShowInMenuBar") as? Bool ?? false } set { d.set(newValue, forKey: "ShowInMenuBar") } }
    /// "Follow external clock rate" (RATE_FOLLOW_PLAN_v1, user 2026-10-22: on by default), for every interface of this Mac
    static var followExternalRate: Bool { get { return d.object(forKey: "FollowExternalRate") as? Bool ?? true } set { d.set(newValue, forKey: "FollowExternalRate") } }

    /// Launch at login = a per-user LaunchAgent that starts the panel with --background (no window until asked; 10.13 has no simpler public API
    /// for an unsandboxed app without a helper bundle).
    static let agentLabel = "com.openaudio.bebobmafw.panel"
    static var agentURL: URL {
        return FileManager.default.homeDirectoryForCurrentUser.appendingPathComponent("Library/LaunchAgents/\(agentLabel).plist")
    }
    static var launchAtLogin: Bool {
        get { return FileManager.default.fileExists(atPath: agentURL.path) }
        set {
            if newValue {
                guard let exe = Bundle.main.executablePath else { return }
                let plist: NSDictionary = ["Label": agentLabel, "ProgramArguments": [exe, "--background"], "RunAtLoad": true, "ProcessType": "Interactive"]
                try? FileManager.default.createDirectory(at: agentURL.deletingLastPathComponent(), withIntermediateDirectories: true, attributes: nil)
                plist.write(to: agentURL, atomically: true)
            } else {
                try? FileManager.default.removeItem(at: agentURL)
            }
        }
    }
}

/// A labelled slider inside a menu: the slider is the fader's travel in the key's law (phones: monitor law, bottom = off; outputs: console law).
final class MenuSliderView: NSView {
    private let label = makeLabel("", size: 12, weight: .medium, color: .labelColor)
    private let readout = makeLabel("", size: 11, color: .secondaryLabelColor)
    let slider = NSSlider(value: 0, minValue: 0, maxValue: 1, target: nil, action: nil)
    var onChange: ((Int) -> Void)?                     // level, half-dB steps
    private let law: FaderLaw

    init(title: String, law: FaderLaw) {
        self.law = law
        super.init(frame: NSRect(x: 0, y: 0, width: 250, height: 46))
        label.stringValue = title
        label.frame = NSRect(x: 20, y: 25, width: 140, height: 16)
        readout.alignment = .right
        readout.frame = NSRect(x: 150, y: 25, width: 82, height: 16)
        slider.frame = NSRect(x: 18, y: 4, width: 216, height: 20)
        slider.isContinuous = true
        slider.target = self; slider.action = #selector(moved)
        [label, readout, slider].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    func show(level: Int?) {
        guard let v = level else { slider.isEnabled = false; readout.stringValue = "-"; return }
        slider.isEnabled = true
        slider.doubleValue = Double(1 - law.fraction(forValue: v))
        readout.stringValue = Theme.levelText(v, law: law) + " dB"
    }
    @objc private func moved() {
        let v = law.value(forFraction: CGFloat(1 - slider.doubleValue))
        readout.stringValue = Theme.levelText(v, law: law) + " dB"
        onChange?(v)
    }
}

final class StatusMenuController: NSObject, NSMenuDelegate {
    /// Units in arrival order, the front panel, and what AppDelegate does for "Open Panel" and a change of the Dock / menu bar settings.
    var units: () -> [PanelWindowController] = { [] }
    var front: () -> PanelWindowController? = { nil }
    var onOpenPanel: ((PanelWindowController?) -> Void)?
    var onPresentationChanged: (() -> Void)?
    let bigMeter: BigMeterController
    private var selectedID: UInt64?
    private var item: NSStatusItem?
    private let menu = NSMenu()
    private var timer: Timer?
    private var shownState: StatusIconState?

    init(bigMeter: BigMeterController) { self.bigMeter = bigMeter; super.init() }

    /// The unit the menu acts on: the one checked in the menu, else the front panel's, else the first.
    var selected: PanelWindowController? {
        let all = units()
        if let id = selectedID, let c = all.first(where: { $0.device.entryID == id }) { return c }
        return front() ?? all.first
    }

    var isInstalled: Bool { return item != nil }

    func install() {
        guard item == nil else { return }
        let it = NSStatusBar.system.statusItem(withLength: NSStatusItem.squareLength)
        menu.delegate = self
        menu.autoenablesItems = false
        // A click opens the panel; a right click (or control-click) opens the menu (user, 2026-10-21). The menu is attached to the item only while
        // it is shown, otherwise every click would open it.
        it.button?.target = self
        it.button?.action = #selector(iconClicked)
        it.button?.sendAction(on: [.leftMouseUp, .rightMouseUp])
        item = it
        shownState = nil
        refreshIcon()
        let t = Timer(timeInterval: 1.0, repeats: true) { [weak self] _ in self?.refreshIcon() }
        RunLoop.main.add(t, forMode: .commonModes)
        timer = t
    }

    @objc private func iconClicked() {
        let e = NSApp.currentEvent
        if e?.type == .rightMouseUp || e?.modifierFlags.contains(.control) == true { showMenu() } else { openPanel() }
    }
    /// The menu under the icon: attached for this click (performClick tracks it until it closes), then detached again.
    private func showMenu() {
        guard let it = item else { return }
        it.menu = menu
        it.button?.performClick(nil)
        it.menu = nil
    }

    /// Screenshot aid (demo `--open-status-menu`): opens the menu as a right click on the icon would.
    func openForCapture() { showMenu() }

    func remove() {
        timer?.invalidate(); timer = nil
        if let it = item { NSStatusBar.system.removeStatusItem(it) }
        item = nil
    }

    func refreshIcon() {
        guard let b = item?.button else { return }
        let s = StatusIconState.of(selected?.ctx.model)
        if s == shownState { return }
        shownState = s
        b.image = StatusMenuController.icon(s)
        b.appearsDisabled = !s.online
        b.toolTip = !s.online ? "OA: no interface" : "OA: \(selected?.device.info.name ?? "")" + (s.noSync ? " - not locked" : "") + (s.muted ? " - muted" : "")
    }

    /// Template image (follows the light / dark menu bar): three level bars; muted adds a slash, no sync an open ring at the top right.
    static func icon(_ s: StatusIconState) -> NSImage {
        let img = NSImage(size: NSSize(width: 18, height: 18), flipped: false) { _ in
            NSColor.black.setFill()
            let bars: [(CGFloat, CGFloat)] = [(2.5, 7), (7, 11), (11.5, 5)]
            for (x, h) in bars { NSBezierPath(roundedRect: NSRect(x: x, y: 3, width: 3, height: h), xRadius: 1, yRadius: 1).fill() }
            if s.muted {
                let p = NSBezierPath(); p.lineWidth = 1.8; p.lineCapStyle = .roundLineCapStyle
                p.move(to: NSPoint(x: 1.5, y: 15)); p.line(to: NSPoint(x: 15.5, y: 1.5))
                NSColor.black.setStroke(); p.stroke()
            }
            if s.noSync {
                let r = NSBezierPath(ovalIn: NSRect(x: 12.5, y: 11.5, width: 5, height: 5)); r.lineWidth = 1.4
                NSColor.black.setStroke(); r.stroke()
            }
            return true
        }
        img.isTemplate = true
        return img
    }

    // MARK: the menu (rebuilt each time it opens). Order (user, 2026-10-21): [units, only with several] Open Panel, the volumes, mute, dim |
    // presets | big meter, Audio MIDI Setup, firmware check | sync source, sample rate, Show Panel In, Launch at Login | rate and lock status,
    // Quit. Headphones A/B and the phones knob step are no longer in the menu (they stay in the panel).

    func menuNeedsUpdate(_ m: NSMenu) {
        m.removeAllItems()
        let all = units()
        let cur = selected
        // The menu acts on the checked unit: the list is needed only when there is a choice
        if all.count > 1 {
            for c in all {
                let i = m.addItem(withTitle: c.displayName.isEmpty ? c.device.info.name : c.displayName, action: #selector(chooseUnit(_:)), keyEquivalent: "")
                i.target = self; i.representedObject = NSNumber(value: c.device.entryID); i.state = (c === cur) ? .on : .off
            }
            m.addItem(NSMenuItem.separator())
        }
        action(m, "Open Panel", #selector(openPanel))
        if let c = cur { levelItems(m, c) }
        m.addItem(NSMenuItem.separator())
        let bm = m.addItem(withTitle: "Big Meter", action: nil, keyEquivalent: ""); bm.submenu = bigMeter.optionsMenu()
        action(m, "Audio MIDI Setup", #selector(openAMS))
        action(m, "Firmware Check\u{2026}", #selector(openFirmwareCheck))
        m.addItem(NSMenuItem.separator())
        if let c = cur { clockItems(m, c) }
        let show = NSMenu()
        let dock = show.addItem(withTitle: "Dock", action: #selector(toggleDock), keyEquivalent: ""); dock.target = self; dock.state = PanelPrefs.showInDock ? .on : .off
        let bar = show.addItem(withTitle: "Menu Bar", action: #selector(toggleMenuBar), keyEquivalent: ""); bar.target = self; bar.state = PanelPrefs.showInMenuBar ? .on : .off
        let showItem = m.addItem(withTitle: "Show Panel In", action: nil, keyEquivalent: ""); showItem.submenu = show
        let login = action(m, "Launch at Login", #selector(toggleLogin)); login.state = PanelPrefs.launchAtLogin ? .on : .off
        m.addItem(NSMenuItem.separator())
        if let model = cur?.ctx.model {
            disabled(m, "\(Theme.rateText(model.sampleRate)) kHz  \u{00B7}  " + (model.rateLocked ? "locked" : "not locked"))
            if let note = cur?.ctx.follower.note { disabled(m, note) }
        } else {
            disabled(m, "No interface connected")
        }
        action(m, "Quit OA BeBoB MAFW", #selector(quit))
    }

    /// Volume sliders (main outputs: every output pair but phones / aux out, console law; phones, monitor law: as their faders, R127), mute, dim,
    /// presets.
    private func levelItems(_ m: NSMenu, _ c: PanelWindowController) {
        let model = c.ctx.model, counts = c.device.profile.counts
        let outCh = model.mainOutputChannels(counts)
        let outs = MenuSliderView(title: "Outputs", law: DeviceModel.law(MAKeys.outputLevels))
        outs.show(level: model.loudest(MAKeys.outputLevels, channels: outCh))
        outs.onChange = { v in for ch in outCh { model.setLevel(MAKeys.outputLevels, channel: ch, value: v, linked: false) } }
        viewItem(m, outs)
        let hpCount = model.ints(MAKeys.headphoneLevels).count
        let hpCh = hpCount > 0 ? Array(1...hpCount) : []
        let phones = MenuSliderView(title: "Phones", law: DeviceModel.law(MAKeys.headphoneLevels))
        phones.show(level: model.loudest(MAKeys.headphoneLevels, channels: hpCh))
        phones.onChange = { v in for ch in hpCh { model.setLevel(MAKeys.headphoneLevels, channel: ch, value: v, linked: false) } }
        viewItem(m, phones)
        let mute = action(m, "Mute", #selector(toggleMute)); mute.state = (model.number(MAKeys.globalMute) ?? 0) != 0 ? .on : .off
        let dim = action(m, "Dim", #selector(toggleDim)); dim.state = (model.number(MAKeys.globalDim) ?? 0) != 0 ? .on : .off
        // presets: the files of this model whose arrays fit the unit (kept, user 2026-10-21)
        let presets = NSMenu(); presets.autoenablesItems = false
        let files = c.presets.fittingFiles()
        if files.isEmpty { disabled(presets, "No presets for this interface") }
        for u in files {
            let i = presets.addItem(withTitle: u.deletingPathExtension().lastPathComponent, action: #selector(loadPreset(_:)), keyEquivalent: "")
            i.target = self; i.representedObject = u; i.state = c.presets.loadedFile == u ? .on : .off
        }
        let presetItem = m.addItem(withTitle: "Presets", action: nil, keyEquivalent: ""); presetItem.submenu = presets
    }

    /// Sync source (with each option's availability) and sample rate (greyed while some client runs the device: rate changes while streaming
    /// are not supported, README known issue 2).
    private func clockItems(_ m: NSMenu, _ c: PanelWindowController) {
        let model = c.ctx.model
        let sync = NSMenu(); sync.autoenablesItems = false
        for opt in c.device.profile.hardware.sync {
            let i = sync.addItem(withTitle: opt.title, action: #selector(chooseSync(_:)), keyEquivalent: "")
            i.target = self; i.representedObject = opt.sync.rawValue; i.state = model.clockSource.sync == opt.sync ? .on : .off
            i.isEnabled = opt.enabled(model.layout)
        }
        let syncItem = m.addItem(withTitle: "Sync Source", action: nil, keyEquivalent: ""); syncItem.submenu = sync
        // RATE_FOLLOW_PLAN_v1: the same setting as the Hardware tab's checkbox (greyed with a driver that does not publish MAExternalSyncRate)
        let follow = action(m, "Follow External Rate", #selector(toggleFollow))
        follow.state = PanelPrefs.followExternalRate ? .on : .off
        follow.isEnabled = c.device.externalSyncRate != nil
        let rates = NSMenu(); rates.autoenablesItems = false
        let busy = model.isStreaming
        for hz in model.supportedRates {
            let i = rates.addItem(withTitle: "\(Theme.rateText(hz)) kHz", action: #selector(chooseRate(_:)), keyEquivalent: "")
            i.target = self; i.representedObject = NSNumber(value: hz); i.state = hz == model.sampleRate ? .on : .off; i.isEnabled = !busy
        }
        if busy { rates.addItem(NSMenuItem.separator()); disabled(rates, "Stop playback to change the rate") }
        let rateItem = m.addItem(withTitle: "Sample Rate", action: nil, keyEquivalent: ""); rateItem.submenu = rates
    }

    @discardableResult private func action(_ m: NSMenu, _ t: String, _ sel: Selector) -> NSMenuItem {
        let i = m.addItem(withTitle: t, action: sel, keyEquivalent: ""); i.target = self; return i
    }
    private func disabled(_ m: NSMenu, _ t: String) { let i = m.addItem(withTitle: t, action: nil, keyEquivalent: ""); i.isEnabled = false }
    private func viewItem(_ m: NSMenu, _ v: NSView) { let i = NSMenuItem(); i.view = v; m.addItem(i) }

    // MARK: actions (on the selected unit)

    @objc private func chooseUnit(_ i: NSMenuItem) { selectedID = (i.representedObject as? NSNumber)?.uint64Value; shownState = nil; refreshIcon() }
    @objc private func toggleMute() { flip(MAKeys.globalMute) }
    @objc private func toggleDim() { flip(MAKeys.globalDim) }
    private func flip(_ k: MAKey) {
        guard let m = selected?.ctx.model else { return }
        m.setNumber(k, (m.number(k) ?? 0) != 0 ? 0 : 1); m.commit()
        shownState = nil; refreshIcon()
    }
    @objc private func chooseRate(_ i: NSMenuItem) {
        guard let m = selected?.ctx.model, let hz = (i.representedObject as? NSNumber)?.uint32Value, !m.isStreaming else { NSSound.beep(); return }
        m.requestSampleRate(hz)
    }
    @objc private func loadPreset(_ i: NSMenuItem) { if let u = i.representedObject as? URL { selected?.presets.loadFile(u) } }
    @objc private func chooseSync(_ i: NSMenuItem) {
        guard let m = selected?.ctx.model, let raw = i.representedObject as? MAClockSource.Sync.RawValue, let s = MAClockSource.Sync(rawValue: raw) else { return }
        m.updateClock { $0.sync = s }
    }
    @objc private func openPanel() { onOpenPanel?(selected) }
    @objc private func openAMS() {
        _ = NSWorkspace.shared.launchApplication(withBundleIdentifier: "com.apple.audio.AudioMIDISetup", options: [], additionalEventParamDescriptor: nil, launchIdentifier: nil)
    }
    /// The firmware check window (formerly the separate OpenAudio Firmware Tool app; merged into the panel, user 2026-10-21).
    var onFirmwareCheck: (() -> Void)?
    @objc private func openFirmwareCheck() { onFirmwareCheck?() }
    @objc private func toggleDock() {
        PanelPrefs.showInDock = !PanelPrefs.showInDock
        if !PanelPrefs.showInDock && !PanelPrefs.showInMenuBar { PanelPrefs.showInMenuBar = true }
        onPresentationChanged?()
    }
    @objc private func toggleMenuBar() {
        PanelPrefs.showInMenuBar = !PanelPrefs.showInMenuBar
        if !PanelPrefs.showInDock && !PanelPrefs.showInMenuBar { PanelPrefs.showInDock = true }
        onPresentationChanged?()
    }
    @objc private func toggleLogin() { PanelPrefs.launchAtLogin = !PanelPrefs.launchAtLogin }
    @objc private func toggleFollow() { PanelPrefs.followExternalRate = !PanelPrefs.followExternalRate }
    @objc private func quit() { NSApp.terminate(nil) }
}
