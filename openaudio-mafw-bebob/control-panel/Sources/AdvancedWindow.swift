// Advanced window (docs/PANEL_REDESIGN_PLAN_v3.md, user 2026-10-27; a clone feature): a second window per unit, opened from the menu bar,
// for the FireWire 1814 at 44.1 / 48 kHz. Header (unit, mode switcher, rate and lock, dim, mute), mode bar (what the mode is and what the
// interface's mixer does in it), and the mode's view. Phase M1: the shell — modes switch with their scenes (AdvancedModes.swift), the views
// themselves come in M2-M5 (docs/panel-redesign/ADVANCED_TODO.md). Outside 44.1 / 48 kHz the window says why and offers the standard panel;
// it never changes the rate or the clock by itself.
import AppKit

/// Header band: unit name, the four modes, the rate pill, dim and mute.
final class AdvancedHeader: FlippedView {
    private(set) var modes: SegmentedBar
    let modeList: [AdvancedMode]
    let dim = PillButton("DIM"), mute = PillButton("MUTE")
    /// The front knob's target and value (single-unit window); hidden in Dual
    let knob = FlatButton("")
    var showsKnob = false { didSet { knob.isHidden = !showsKnob; relayout() } }
    var knobText = "" { didSet { if knobText != oldValue { knob.title = knobText; relayout() } } }
    var name = "" { didSet { relayout() } }
    var subtitle = "" { didSet { if subtitle != oldValue { relayout() } } }
    var rate = "" { didSet { relayout() } }
    var locked = true { didSet { needsDisplay = true } }
    var modesEnabled = true { didSet { modes.alphaValue = modesEnabled ? 1 : Theme.disabledAlpha } }
    private var pillRect = NSRect.zero
    private static let nameFont = Theme.font(14, .semibold), pillFont = Theme.font(12, .medium)

    init(modes list: [AdvancedMode] = AdvancedMode.all) {
        modeList = list
        modes = SegmentedBar(items: list.map { .text($0.name) }, segmentWidth: 96, tooltips: list.map { $0.title })
        super.init(frame: .zero)
        for (b, c) in [(dim, Theme.green), (mute, Theme.mute)] {
            b.solid = true; b.onColor = c; b.fontSize = 11.5
            addSubview(b)
        }
        addSubview(modes)
        knob.isHidden = true
        knob.toolTip = "What the front knob of the 1814 drives (click to choose)"
        addSubview(knob)
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    /// M6: the segments change when custom modes come and go (the built-in four, the user's modes, then ＋)
    func setModeNames(_ names: [String], tooltips: [String]) {
        let handler = modes.onSelect, sel = modes.selected
        modes.removeFromSuperview()
        modes = SegmentedBar(items: names.map { .text($0) }, segmentWidth: names.count > 6 ? 84 : 96, tooltips: tooltips)
        modes.onSelect = handler
        modes.selected = min(sel, names.count - 1)
        modes.alphaValue = modesEnabled ? 1 : Theme.disabledAlpha
        addSubview(modes)
        relayout()
    }

    override func setFrameSize(_ newSize: NSSize) { super.setFrameSize(newSize); relayout() }

    private func relayout() {
        let w = bounds.width, y = ((bounds.height - 30) / 2).rounded()
        let nameW = max(Theme.textWidth(name, font: AdvancedHeader.nameFont), Theme.textWidth(subtitle, font: Theme.font(10.5)), 150)   // the subtitle never runs under the modes
        modes.setFrameOrigin(NSPoint(x: Theme.pad + nameW + 24, y: y))
        mute.frame = NSRect(x: w - Theme.pad - 64, y: y, width: 64, height: 30)
        dim.frame = NSRect(x: mute.frame.minX - 8 - 56, y: y, width: 56, height: 30)
        let pw = Theme.textWidth(rate, font: AdvancedHeader.pillFont) + 34
        pillRect = NSRect(x: dim.frame.minX - 12 - pw, y: y, width: pw, height: 30)
        if showsKnob {
            let kw = knob.intrinsicContentSize.width + 8
            knob.frame = NSRect(x: max(modes.frame.maxX + 12, pillRect.minX - 12 - kw), y: y + 2, width: kw, height: 26)
        }
        needsDisplay = true
    }

    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) {
        Theme.chrome.setFill(); dirtyRect.fill()
        let x = Theme.pad + 2
        Theme.draw(name, in: NSRect(x: x, y: bounds.midY - 17, width: 260, height: 18), font: AdvancedHeader.nameFont, color: Theme.text, align: .left)
        Theme.draw(subtitle, in: NSRect(x: x, y: bounds.midY + 1, width: 260, height: 14), font: Theme.font(10.5), color: Theme.text3, align: .left)
        Theme.fill(pillRect, radius: 15, Theme.raised)
        (locked ? Theme.green : Theme.solo).setFill()
        NSBezierPath(ovalIn: NSRect(x: pillRect.minX + 12, y: pillRect.midY - 3.5, width: 7, height: 7)).fill()
        Theme.draw(rate, in: NSRect(x: pillRect.minX + 26, y: pillRect.minY, width: pillRect.width - 30, height: pillRect.height),
                   font: AdvancedHeader.pillFont, color: Theme.text2, align: .left)
    }
}

/// Mode bar under the header: the mode's title and summary on the left, the interface's state on the right.
final class AdvancedModeBar: FlippedView {
    var title = "" { didSet { needsDisplay = true } }
    var summary = "" { didSet { needsDisplay = true; updateTip() } }
    var state = "" { didSet { needsDisplay = true; updateTip() } }
    /// the whole text as a tooltip: the note, summary and state are cut when the window is narrow
    private func updateTip() { toolTip = [title, note ?? summary, state].filter { !$0.isEmpty }.joined(separator: "\n") }
    /// Right-hand status after the state: "applied" (green), "edited" (amber: changed since the mode applied its scene; kept in the mode),
    /// or none (the lock-out)
    enum Status { case applied, edited, none }
    var status = Status.applied { didSet { needsDisplay = true } }
    var note: String? { didSet { needsDisplay = true; updateTip() } }          // a one-off message (first opening) in place of the summary
    /// Dual window: the chip's status text and colour in place of applied / edited, and a click on the chip
    var statusOverride: (String, NSColor)? { didSet { needsDisplay = true } }
    var onChipClick: (() -> Void)?
    private var chipRect = NSRect.zero
    override func mouseDown(with e: NSEvent) { if chipRect.contains(convert(e.locationInWindow, from: nil)) { onChipClick?() } }

    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) {
        Theme.hex(0x0D0E10).setFill(); bounds.fill()
        Theme.hairline.setFill()
        NSRect(x: 0, y: 0, width: bounds.width, height: 1).fill()
        NSRect(x: 0, y: bounds.height - 1, width: bounds.width, height: 1).fill()
        let tf = Theme.font(12.5, .semibold)
        let tw = Theme.textWidth(title, font: tf)
        Theme.draw(title, in: NSRect(x: Theme.pad, y: 0, width: tw + 4, height: bounds.height), font: tf, color: Theme.text, align: .left)
        let statusText: String
        switch status { case .applied: statusText = "\u{2713} applied"; case .edited: statusText = "edited \u{2014} kept in this mode"; case .none: statusText = "" }
        let shownStatus = statusOverride?.0 ?? statusText
        let sf = Theme.font(11)
        let cw = Theme.textWidth(state, font: sf) + Theme.textWidth(shownStatus, font: sf) + (shownStatus.isEmpty ? 20 : 30)
        let chip = NSRect(x: bounds.width - Theme.pad - cw, y: (bounds.height - 22) / 2, width: cw, height: 22)
        chipRect = chip
        let left = note ?? (summary.isEmpty ? "" : "\u{2014} " + summary)
        Theme.draw(left, in: NSRect(x: Theme.pad + tw + 12, y: 0, width: max(0, chip.minX - Theme.pad - tw - 24), height: bounds.height),
                   font: Theme.font(11.5), color: note != nil ? Theme.solo : Theme.text2, align: .left)
        Theme.fill(chip, radius: 11, Theme.raised)
        let sw = Theme.textWidth(state, font: sf)
        Theme.draw(state, in: NSRect(x: chip.minX + 10, y: chip.minY, width: sw + 2, height: chip.height), font: sf, color: Theme.text2, align: .left)
        Theme.draw(shownStatus, in: NSRect(x: chip.minX + 18 + sw, y: chip.minY, width: chip.maxX - chip.minX - sw - 22, height: chip.height),
                   font: sf, color: statusOverride?.1 ?? (status == .edited ? Theme.solo : Theme.green), align: .left)
    }
}

/// Root of the window: header, mode bar, body.
final class AdvancedRoot: FlippedView {
    let header: AdvancedHeader, bar = AdvancedModeBar(frame: .zero), body = FlippedView(frame: .zero)
    static let barHeight: CGFloat = 38
    init(modes: [AdvancedMode] = AdvancedMode.all) {
        header = AdvancedHeader(modes: modes)
        super.init(frame: .zero)
        [header, bar, body].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) { Theme.window.setFill(); dirtyRect.fill() }
    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        let h = Theme.headerHeight, b = AdvancedRoot.barHeight
        header.frame = NSRect(x: 0, y: 0, width: newSize.width, height: h)
        bar.frame = NSRect(x: 0, y: h, width: newSize.width, height: b)
        body.frame = NSRect(x: 0, y: h + b, width: newSize.width, height: max(0, newSize.height - h - b))
        body.subviews.forEach { $0.frame = body.bounds }
    }
}

/// A view of the body: a mode's view (M2-M5) or the lock-out.
protocol AdvancedView: AnyObject {
    var view: NSView { get }
    func refresh()
    func updateMeters()
}

/// Card in the middle of the body with a heading, text and optional buttons (placeholders of M1 and the lock-out).
final class AdvancedMessageView: FlippedView, AdvancedView {
    var view: NSView { return self }
    private let card = Card(title: nil)
    private let heading = makeLabel("", size: 16, weight: .semibold, color: Theme.text)
    private let text = makeLabel("", size: 12.5, color: Theme.text2)
    private(set) var buttons: [FlatButton] = []
    private let refreshBody: ((AdvancedMessageView) -> Void)?

    init(heading h: String, text t: String, buttons bs: [FlatButton] = [], refresh: ((AdvancedMessageView) -> Void)? = nil) {
        refreshBody = refresh
        super.init(frame: .zero)
        heading.stringValue = h
        text.stringValue = t
        text.maximumNumberOfLines = 6
        text.lineBreakMode = .byWordWrapping
        text.cell?.wraps = true
        buttons = bs
        addSubview(card)
        [heading, text].forEach { card.addSubview($0) }
        bs.forEach { card.addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    func setText(_ t: String) { text.stringValue = t }
    func refresh() { refreshBody?(self) }
    func updateMeters() {}

    override var isOpaque: Bool { return false }
    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        let w: CGFloat = 580, h: CGFloat = buttons.isEmpty ? 150 : 200
        card.frame = NSRect(x: ((newSize.width - w) / 2).rounded(), y: max(24, ((newSize.height - h) / 2 - 30).rounded()), width: w, height: h)
        heading.frame = NSRect(x: 26, y: 24, width: w - 52, height: 22)
        text.frame = NSRect(x: 26, y: 54, width: w - 52, height: 84)
        var x: CGFloat = 26
        for b in buttons {
            let bw = b.intrinsicContentSize.width
            b.frame = NSRect(x: x, y: h - 26 - 28, width: bw, height: 28)
            x += bw + 10
        }
    }
}

final class AdvancedWindowController: NSObject, NSWindowDelegate {
    let ctx: PanelContext
    let device: PanelBackend
    let window: NSWindow
    let store: AdvancedStore
    private let root = AdvancedRoot()
    private(set) var mode: AdvancedMode = AdvancedMode.first
    /// The current mode's key: a built-in mode's rawValue or a custom mode's id (M6); `mode` is its kind
    private(set) var modeKey = AdvancedMode.first.rawValue
    private var applied: [String: Any]?             // the scene the current mode last wrote (for "edited")
    private var headerKeys: [String] = []
    private var bodyView: AdvancedView?
    private var bodyKey = ""                          // which view is in the body ("lock", or a mode's rawValue)
    private var noteTimer: Timer?
    /// "Open the standard panel" (the lock-out's button)
    var onOpenStandard: (() -> Void)?
    var displayName = "" { didSet { applyName() } }
    private func applyName() { root.header.name = displayName; window.title = "\(displayName) \u{2014} Advanced" }

    static let contentSize = NSSize(width: 1280, height: 720)
    /// Content size for the screen (user, 2026-10-28: made for Full HD): up to 1880 x 1000, never under 1280 x 720 (the 11-inch Air's room).
    static func contentSize(for screen: NSScreen?) -> NSSize {
        guard let vf = (screen ?? NSScreen.main)?.visibleFrame else { return contentSize }
        return NSSize(width: min(1880, max(contentSize.width, vf.width - 60)), height: min(1000, max(contentSize.height, vf.height - 80)))
    }
    /// Sizes a window to the screen the first time it opens (later the user's size is kept: the windows are resizable).
    static func fit(_ w: NSWindow) {
        w.contentMinSize = contentSize
        w.setContentSize(contentSize(for: w.screen ?? NSScreen.main))
    }

    init(context: PanelContext, device: PanelBackend, name: String, storeFolder: URL = AdvancedStore.folder) {
        ctx = context
        self.device = device
        store = AdvancedStore(guid: device.info.guid, folder: storeFolder)
        window = NSWindow(contentRect: NSRect(origin: .zero, size: AdvancedWindowController.contentSize),
                          styleMask: [.titled, .closable, .miniaturizable, .resizable], backing: .buffered, defer: false)
        super.init()
        window.isReleasedWhenClosed = false
        window.delegate = self
        styleWindow(window)
        // GPU-first rule, measured 2026-10-28 (Dual DAW demo on the iMac): one layer per view made Core Animation walk ~1000 layers on every
        // meter commit. The static views draw into the root's one layer (once, then only where they change); only the moving parts (meter
        // bars, columns with a peak readout) keep layers of their own, by asking for one explicitly.
        root.wantsLayer = true
        root.canDrawSubviewsIntoLayer = true
        window.contentView = root
        AdvancedWindowController.fit(window)
        root.header.modes.onSelect = { [weak self] i in self?.selectSegment(i) }
        NotificationCenter.default.addObserver(self, selector: #selector(writeFailed), name: DeviceModel.writeFailed, object: context.model)
        root.header.dim.onClick = { [weak self] b in self?.setGlobal(MAKeys.globalDim, b.isOn) }
        root.header.knob.onAction = { [weak self] in self?.knobMenu() }
        root.header.mute.onClick = { [weak self] b in self?.setGlobal(MAKeys.globalMute, b.isOn) }
        if let k = store.modeKey, let m = store.kind(of: k) { mode = m; modeKey = k; applied = store.scene(key: k) }
        displayName = name
        applyName()                                     // property observers do not run inside init
        refresh()
    }

    var available: Bool { return AdvancedGate.available(modelAllows: device.profile.advancedPanel, rate: ctx.model.sampleRate) }

    func show() {
        if !window.isVisible { placeWindow(window, center: screenCenter()) }
        firstOpenIfNeeded()
        refresh()
        window.makeKeyAndOrderFront(nil)
    }
    func close() { noteTimer?.invalidate(); window.orderOut(nil) }
    func hide() { window.orderOut(nil) }
    /// Snapshot aid (`--demo --advanced --snapshot`): a window opened inside the snapshot run is not reported visible yet, so meters run anyway
    var ignoresOcclusion = false
    var isShown: Bool { return window.isVisible && (benchIgnoresOcclusion || ignoresOcclusion || window.occlusionState.contains(.visible)) }

    /// The first time the window opens for a unit (and only at a gated rate): today's mixer is kept as the Monitor scene, then DAW is applied.
    /// DAW is a passthrough that keeps every output level, so nothing gets louder (AdvancedScene.passthrough).
    func firstOpenIfNeeded() {
        guard store.mode == nil, available else { return }
        let cur = ctx.model.cache
        store.store(scene: AdvancedScene.capture(cur), for: .monitor)
        let target = AdvancedScene.template(.daw, current: cur)
        ctx.model.restore(target)
        store.store(mode: .daw, scene: target)
        mode = .daw; modeKey = AdvancedMode.daw.rawValue; applied = target
        showNote("Opened in DAW: mixer set to passthrough; your previous mixer is kept as the Monitor mode.")
    }

    private func showNote(_ s: String) {
        root.bar.note = s
        noteTimer?.invalidate()
        noteTimer = Timer.scheduledTimer(withTimeInterval: 8, repeats: false) { [weak self] _ in self?.root.bar.note = nil }
    }

    // MARK: model -> view

    func refresh() {
        let m = ctx.model, info = device.info
        let h = root.header
        let sync = device.profile.hardware.sync.first(where: { $0.sync == m.clockSource.sync })?.title ?? ""
        let syncShort = sync.components(separatedBy: " - ").first ?? sync
        let ports = device.profile.hardware.digitalPorts ? " \u{00B7} " + (m.clockSource.adatIn ? "ADAT" : "S/PDIF") + " in" : ""
        h.rate = "\(Theme.rateText(m.sampleRate)) kHz  \u{00B7}  \(syncShort)\(ports)  \u{00B7}  " + (m.rateLocked ? "locked" : "not locked")
        h.locked = m.rateLocked
        h.subtitle = "Advanced \u{00B7} \(String(info.guid.suffix(6)))"
        h.dim.isOn = (m.number(MAKeys.globalDim) ?? 0) != 0
        h.mute.isOn = (m.number(MAKeys.globalMute) ?? 0) != 0
        h.modesEnabled = available
        h.showsKnob = available
        h.knobText = "Knob \u{00B7} " + KnobTarget.describe(m.rotaryMask) + knobValue(m) + " \u{25BE}"
        if headerKeys != store.modeKeys {
            headerKeys = store.modeKeys
            h.setModeNames(headerKeys.map { store.modeName($0) } + ["\u{FF0B}"],
                           tooltips: headerKeys.map { k in store.kind(of: k).map { m in store.custom(k) == nil ? m.title : "Your mode, based on \(m.name): \(m.title)" } ?? "" }
                               + ["New mode from this one, rename or delete yours"])
        }
        h.modes.selected = headerKeys.index(of: modeKey) ?? 0
        let bar = root.bar
        if available {
            if let c = store.custom(modeKey) { bar.title = c.name; bar.summary = mode.name + " \u{00B7} " + mode.title }
            else { bar.title = mode.title; bar.summary = mode.summary }
            bar.state = mode.interfaceState
            let nh = store.hiddenIDs(modeKey).count
            if nh > 0 { bar.summary += " \u{00B7} \(nh) channel\(nh == 1 ? "" : "s") hidden (right-click a channel to show)" }
            let edited = applied.map { !NSDictionary(dictionary: AdvancedScene.capture(m.cache)).isEqual(to: $0) } ?? false
            bar.status = edited ? .edited : .applied
            bar.statusOverride = m.writeFailure.map { f in
                (String(format: "\u{26A0} the driver refused %d setting%@ (0x%x)", f.keys.count, f.keys.count == 1 ? "" : "s", f.code), Theme.red)
            }
        } else {
            bar.title = "Advanced"; bar.summary = "the FireWire 1814 at 44.1 / 48 kHz"; bar.state = "Outside 44.1 / 48 kHz: the standard panel only"; bar.status = .none
        }
        store.viewKey = modeKey
        setBody(available ? modeKey : "lock")
        bodyView?.refresh()
    }

    func updateMeters() { bodyView?.updateMeters() }

    private func setBody(_ key: String) {
        guard key != bodyKey else { return }
        bodyKey = key
        root.body.subviews.forEach { $0.removeFromSuperview() }
        let v: AdvancedView
        switch key == "lock" ? key : (store.kind(of: key) ?? .daw).rawValue {
        case "lock": v = lockView()
        case AdvancedMode.daw.rawValue: v = AdvancedDAWView(context: ctx, store: store)
        case AdvancedMode.console.rawValue: v = AdvancedConsoleView(context: ctx, store: store)
        case AdvancedMode.monitor.rawValue: v = AdvancedMonitorView(context: ctx, store: store)
        case AdvancedMode.hybrid.rawValue: v = AdvancedHybridView(context: ctx, store: store)
        default: v = placeholder(AdvancedMode(rawValue: key) ?? .daw)
        }
        bodyView = v
        v.view.frame = root.body.bounds
        root.body.addSubview(v.view)
    }

    /// M1: the views come in M2 (DAW), M3 (Console), M4 (Monitor), M5 (Hybrid); the mode's scene is already applied.
    private func placeholder(_ m: AdvancedMode) -> AdvancedView {
        let phase: String
        switch m { case .daw: phase = "M2"; case .console: phase = "M3"; case .monitor: phase = "M4"; case .hybrid: phase = "M5" }
        return AdvancedMessageView(heading: "\(m.name) \u{2014} \(m.title)",
                                   text: "This mode's interface scene is applied (\(m.interfaceState)). Its view is the next step of the plan (phase \(phase), docs/panel-redesign/ADVANCED_TODO.md).")
    }

    private func lockView() -> AdvancedView {
        let open = FlatButton("Open the standard panel"); open.primary = true
        open.onAction = { [weak self] in self?.onOpenStandard?() }
        let set48 = FlatButton("Set 48 kHz")
        set48.onAction = { [weak self] in
            guard let m = self?.ctx.model, !m.isStreaming else { NSSound.beep(); return }
            m.requestSampleRate(48000)
        }
        let text = { [weak self] () -> String in
            guard let me = self else { return "" }
            let m = me.ctx.model
            let busy = m.isStreaming ? " Stop playback before changing the rate." : ""
            return "The interface runs at \(Theme.rateText(m.sampleRate)) kHz. The Advanced window is for the FireWire 1814 at 44.1 and 48 kHz, where it has its full cue mixer and eight ADAT channels each way. The standard panel works at every rate. Nothing here changes the rate by itself.\(busy)"
        }
        return AdvancedMessageView(heading: "Advanced works at 44.1 and 48 kHz", text: text(), buttons: [open, set48]) { [weak self] v in
            v.setText(text())
            v.buttons.last?.isEnabled = !(self?.ctx.model.isStreaming ?? true)
        }
    }

    // MARK: mode switching

    /// The switch the user asked for: save the mode left, apply the target's scene; ask first when an output or a phone would get louder.
    func request(_ m: AdvancedMode, ask: Bool = true) { request(key: m.rawValue, ask: ask) }
    func request(key k: String, ask: Bool = true) {
        guard available, let to = store.kind(of: k) else { refresh(); return }
        guard k != modeKey else { return }
        let plan = AdvancedSwitch.plan(from: mode, to: to, current: ctx.model.cache, stored: store.scene(key: k))
        if ask && !plan.rises.isEmpty {
            let alert = AdvancedWindowController.riseAlert(plan, title: store.modeName(k))
            alert.beginSheetModal(for: window) { [weak self] r in
                if r == .alertFirstButtonReturn { self?.apply(plan, toKey: k) } else { self?.refresh() }
            }
            return
        }
        apply(plan, toKey: k)
    }

    /// The modes the status menu lists: key, name, current (M6, plan v2 §3)
    var modeEntries: [(key: String, name: String, current: Bool)] {
        return store.modeKeys.map { ($0, store.modeName($0), store.modeKey != nil && $0 == modeKey) }
    }

    private func selectSegment(_ i: Int) {
        if i < headerKeys.count { request(key: headerKeys[i]); return }
        root.header.modes.selected = headerKeys.index(of: modeKey) ?? 0          // ＋ is a menu, not a mode
        plusMenu()
    }

    // MARK: custom modes (M6)

    private func plusMenu() {
        let menu = NSMenu(); menu.autoenablesItems = false
        let cur = store.modeName(modeKey)
        func item(_ t: String, enabled: Bool = true, _ f: @escaping () -> Void) {
            let a = MenuAction(f)
            let i = NSMenuItem(title: t, action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
            i.target = a; i.representedObject = a; i.isEnabled = enabled
            menu.addItem(i)
        }
        item("New mode from \u{201C}\(cur)\u{201D}\u{2026}", enabled: available) { [weak self] in self?.newCustomMode() }
        let isCustom = store.custom(modeKey) != nil
        item("Rename \u{201C}\(cur)\u{201D}\u{2026}", enabled: isCustom) { [weak self] in self?.renameCustomMode() }
        item("Delete \u{201C}\(cur)\u{201D}", enabled: isCustom) { [weak self] in self?.deleteCustomMode() }
        let bar = root.header.modes
        menu.popUp(positioning: nil, at: NSPoint(x: bar.bounds.maxX - 40, y: bar.bounds.maxY + 4), in: bar)
    }
    private func askName(_ title: String, _ current: String, _ done: @escaping (String) -> Void) {
        let a = NSAlert()
        a.messageText = title
        a.informativeText = "A mode keeps its own interface scene, channel names shown and hidden channels."
        let f = NSTextField(frame: NSRect(x: 0, y: 0, width: 240, height: 24)); f.stringValue = current
        a.accessoryView = f; a.window.initialFirstResponder = f
        a.addButton(withTitle: "OK"); a.addButton(withTitle: "Cancel")
        a.beginSheetModal(for: window) { r in if r == .alertFirstButtonReturn { done(f.stringValue) } }
    }
    /// Duplicates the current mode: today's mixer becomes the new mode's scene (nothing is written to the interface)
    private func newCustomMode() {
        askName("New mode from \u{201C}\(store.modeName(modeKey))\u{201D}", store.modeName(modeKey) + " 2") { [weak self] n in
            guard let me = self else { return }
            let scene = AdvancedScene.capture(me.ctx.model.cache)
            me.store.store(scene: scene, key: me.modeKey)
            guard let k = me.store.addCustom(name: n, from: me.modeKey, scene: scene) else { return }
            me.store.store(modeKey: k, scene: nil)
            me.modeKey = k; me.applied = scene
            me.refresh()
        }
    }
    private func renameCustomMode() {
        guard let c = store.custom(modeKey) else { return }
        askName("Rename \u{201C}\(c.name)\u{201D}", c.name) { [weak self] n in
            self?.store.renameCustom(c.id, to: n); self?.headerKeys = []; self?.refresh()
        }
    }
    /// Deletes the current custom mode and goes to its base mode (asking first if that would make an output louder)
    private func deleteCustomMode() {
        guard let c = store.custom(modeKey) else { return }
        let a = NSAlert()
        a.messageText = "Delete \u{201C}\(c.name)\u{201D}?"
        a.informativeText = "Its scene and view options are removed; the window goes to \(c.base.name)."
        a.addButton(withTitle: "Delete"); a.addButton(withTitle: "Cancel")
        a.beginSheetModal(for: window) { [weak self] r in
            guard r == .alertFirstButtonReturn, let me = self else { return }
            let plan = AdvancedSwitch.plan(from: me.mode, to: c.base, current: me.ctx.model.cache, stored: me.store.scene(c.base))
            me.ctx.model.clearWriteFailure()
            me.ctx.model.restore(plan.applying)
            me.store.removeCustom(c.id)
            me.store.store(modeKey: c.base.rawValue, scene: plan.applying)
            me.mode = c.base; me.modeKey = c.base.rawValue; me.applied = plan.applying
            me.refresh()
        }
    }

    /// The switch for `m` without asking (the Dual window asks once for both units).
    func plan(_ m: AdvancedMode) -> AdvancedSwitch {
        return AdvancedSwitch.plan(from: mode, to: m, current: ctx.model.cache, stored: store.scene(key: m.rawValue))
    }

    @objc private func writeFailed() { if window.isVisible { refresh() } }

    // MARK: the front knob (header chip)

    /// " · -12.0 dB" for a knob on output pairs (the monitor level it moves), else nothing
    private func knobValue(_ m: DeviceModel) -> String {
        let mask = m.rotaryMask
        guard MARotaryMask.group(of: mask) == .output, let first = KnobTarget.items.first(where: { $0.group == .output && mask & $0.bit != 0 }) else { return "" }
        let ch = first.bit == MARotaryMask.outputPairBit(2) ? 3 : 1
        return " \u{00B7} " + Theme.levelText(m.faderLevel(MAKeys.outputLevels, channel: ch)) + " dB"
    }
    private func knobMenu() {
        let m = ctx.model, mask = m.rotaryMask
        let menu = NSMenu(); menu.autoenablesItems = false
        let head = NSMenuItem(title: "The front knob drives (one group at a time):", action: nil, keyEquivalent: ""); head.isEnabled = false
        menu.addItem(head)
        var last: MARotaryMask.Group?
        for it in KnobTarget.items {
            if let l = last, l != it.group { menu.addItem(.separator()) }
            last = it.group
            let a = MenuAction { [weak self] in
                m.setNumber(MAKeys.rotaryControl, KnobTarget.choose(it, current: m.rotaryMask)); m.commit(); self?.refresh()
            }
            let i = NSMenuItem(title: it.title, action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
            i.target = a; i.representedObject = a; i.state = mask & it.bit == it.bit ? .on : .off
            menu.addItem(i)
        }
        menu.addItem(.separator())
        let none = MenuAction { [weak self] in m.setNumber(MAKeys.rotaryControl, 0); m.commit(); self?.refresh() }
        let ni = NSMenuItem(title: "Nothing", action: #selector(MenuAction.fire(_:)), keyEquivalent: "")
        ni.target = none; ni.representedObject = none; ni.state = mask == 0 ? .on : .off
        menu.addItem(ni)
        let k = root.header.knob
        menu.popUp(positioning: nil, at: NSPoint(x: 0, y: k.bounds.height + 4), in: k)
    }

    func apply(_ p: AdvancedSwitch) { apply(p, toKey: p.to.rawValue) }
    /// The scene left is kept under the current mode's key, the target's under its key (a custom mode or the built-in one)
    func apply(_ p: AdvancedSwitch, toKey k: String) {
        store.store(scene: p.leaving, key: modeKey)
        ctx.model.clearWriteFailure()
        ctx.model.restore(p.applying)
        store.store(modeKey: k, scene: p.applying)
        mode = p.to; modeKey = k; applied = p.applying
        refresh()
    }

    /// The question before a switch that raises levels (plan v2 §3): which outputs, from and to.
    static func riseAlert(_ p: AdvancedSwitch, title modeName: String? = nil) -> NSAlert {
        let a = NSAlert()
        a.messageText = "Switch to \(modeName ?? p.to.name)?"
        func name(_ r: LevelRise) -> String {
            if r.key == MAKeys.headphoneLevels.name { return "Phones \((r.channel + 1) / 2) " + (r.channel % 2 == 1 ? "L" : "R") }
            return "Line out \(r.channel)"
        }
        let law: (String) -> FaderLaw = { $0 == MAKeys.headphoneLevels.name ? .monitor : .console }
        let lines = p.rises.map { "\(name($0)): \(Theme.levelText($0.from, law: law($0.key))) \u{2192} \(Theme.levelText($0.to, law: law($0.key))) dB" }
        a.informativeText = "These outputs get louder:\n" + lines.joined(separator: "\n") + "\n\nRate and clock are not changed."
        a.addButton(withTitle: "Switch")
        a.addButton(withTitle: "Cancel")
        return a
    }

    private func setGlobal(_ k: MAKey, _ on: Bool) {
        ctx.model.setNumber(k, on ? 1 : 0); ctx.model.commit()
    }

    /// Closing hides the window; it comes back from the menu bar.
    func windowShouldClose(_ sender: NSWindow) -> Bool { window.orderOut(nil); return false }
}
