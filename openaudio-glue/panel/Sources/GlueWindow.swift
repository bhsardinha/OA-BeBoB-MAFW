// OpenAudio Glue app - the window (GLUE_PLAN_v1.md phase 4): header (unit tabs, the unit's bypass, state, Dual Glue aggregate),
// the global colour card (the four stage selectors) and the channel matrix, 6 cards per row (user, 2026-10-28). Settings go to the
// device as one plist, at most every 33 ms while dragging; meters are read at 15 Hz.
import AppKit

final class GlueRootView: FlippedView {
    override func draw(_ dirtyRect: NSRect) {
        Theme.window.setFill(); bounds.fill()
        Theme.chrome.setFill(); NSRect(x: 0, y: 0, width: bounds.width, height: Theme.headerHeight).fill()
    }
}

final class GlueWindowController: NSObject, NSWindowDelegate {
    static let perRow = 6
    let window: NSWindow
    private let backend: GlueBackend
    private let root = GlueRootView()
    private let scroll = NSScrollView()
    private let matrix = FlippedView()
    private let globalCard = Card(title: "GLOBAL COLOUR  \u{00B7}  every channel of this unit")
    private var stageBars: [SegmentedBar] = []
    private var unitBar: SegmentedBar?
    private let bypass = PillButton("BYPASS")
    private let drive = ThresholdSlider()
    private let driveCaption = makeLabel("DRIVE", size: 9, weight: .medium, color: Theme.text3)
    private let driveValue = makeLabel("0.0 dB", size: 11, color: Theme.text2, align: .right)
    private let status = makeLabel("", size: 11, color: Theme.text3)
    private let title = makeLabel("OpenAudio Glue", size: 14, weight: .semibold, color: Theme.text)
    private let dualButton = FlatButton("Create Dual Glue")
    private let empty = makeLabel("No Glue device. Glue needs the OpenAudio driver with the Glue tap and the Glue plug-in.", size: 12, color: Theme.text3)
    private var cards: [ChannelCard] = []
    private var unitIndex = 0
    private var settings = GlueSettings()
    private var dirty = false
    private var timers: [Timer] = []

    init(backend: GlueBackend) {
        self.backend = backend
        let width = Theme.pad * 2 + CGFloat(GlueWindowController.perRow) * ChannelCard.width + CGFloat(GlueWindowController.perRow - 1) * 8
        window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: width, height: 900), styleMask: [.titled, .closable, .miniaturizable, .resizable],
                          backing: .buffered, defer: false)
        super.init()
        window.title = "OpenAudio Glue"
        window.appearance = NSAppearance(named: .vibrantDark)
        window.backgroundColor = Theme.chrome
        window.minSize = NSSize(width: width, height: 480)
        window.delegate = self
        root.wantsLayer = true
        window.contentView = root
        for v in [title, status, bypass, dualButton, globalCard, empty] as [NSView] { root.addSubview(v) }
        bypass.onColor = Theme.solo
        bypass.solid = true
        bypass.onClick = { [weak self] b in guard let s = self else { return }; s.settings.bypass = b.isOn; s.dirty = true }
        dualButton.onAction = { [weak self] in self?.toggleDual() }
        drive.range = 0...18
        drive.fillFromLeft = true
        drive.toolTip = "Drive: pushes every colour stage harder (0 to +18 dB), with the level kept about the same"
        drive.onChange = { [weak self] v in guard let s = self else { return }; s.settings.drive = v; s.updateDriveLabel(); s.dirty = true }
        for v in [drive, driveCaption, driveValue] as [NSView] { globalCard.addSubview(v) }
        for (k, name) in GlueOptions.stageNames.enumerated() {
            let items = GlueOptions.options(k).map { SegmentedBar.Item.text($0) }
            let bar = SegmentedBar(items: items, segmentWidth: k == GlueOptions.deskStage ? 58 : 62)
            bar.onSelect = { [weak self] i in guard let s = self else { return }; s.settings.stages[k] = i; s.dirty = true }
            globalCard.addSubview(bar)
            stageBars.append(bar)
            let cap = makeLabel(name.uppercased(), size: 9, weight: .medium, color: Theme.text3)
            cap.tag = 100 + k
            globalCard.addSubview(cap)
        }
        scroll.drawsBackground = false
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true
        scroll.documentView = matrix
        root.addSubview(scroll)
        backend.onUnitsChanged = { [weak self] in self?.reloadUnits() }
        reloadUnits()
        timers.append(Timer.scheduledTimer(withTimeInterval: 1.0 / 30, repeats: true) { [weak self] _ in self?.flush() })
        timers.append(Timer.scheduledTimer(withTimeInterval: 1.0 / 15, repeats: true) { [weak self] _ in self?.updateMeters() })
        window.center()
    }


    // MARK: units
    private func unitTitle(_ u: GlueUnit, _ i: Int) -> String {
        let tail = String(u.guidHex.suffix(4))
        return backend.units.count > 1 ? "\(i == 0 ? "A" : "B")  \u{00B7}  \u{2026}\(tail)" : "\u{2026}\(tail)"
    }

    /// Units in the main panel's Dual order (A first) when there is one, else by GUID.
    private func orderedUnits() -> [GlueUnit] {
        let order = backend.dualOrder()
        return backend.units.sorted { a, b in
            let ia = order.index(of: a.guidHex) ?? Int.max, ib = order.index(of: b.guidHex) ?? Int.max
            return ia != ib ? ia < ib : a.guidHex < b.guidHex
        }
    }

    private func reloadUnits() {
        unitBar?.removeFromSuperview()
        let units = orderedUnits()
        if units.count > 1 {
            let bar = SegmentedBar(items: units.enumerated().map { SegmentedBar.Item.text(unitTitle($0.element, $0.offset)) }, segmentWidth: 120)
            bar.onSelect = { [weak self] i in self?.selectUnit(i) }
            root.addSubview(bar)
            unitBar = bar
        } else {
            unitBar = nil
        }
        unitIndex = min(unitIndex, max(0, units.count - 1))
        selectUnit(unitIndex)
    }

    private func selectUnit(_ i: Int) {
        flush()
        unitIndex = i
        unitBar?.selected = i
        let units = orderedUnits()
        guard i < units.count else { buildCards(nil); return }
        let u = units[i]
        settings = backend.settings(u)
        buildCards(u)
    }

    private func currentUnit() -> GlueUnit? { let u = orderedUnits(); return unitIndex < u.count ? u[unitIndex] : nil }

    // MARK: cards
    private func labelKey(_ u: GlueUnit, _ c: Int) -> String { return "label.\(u.guidHex).\(c)" }

    private func routing(_ u: GlueUnit, _ c: Int) -> String {
        var r = "In \(c + 1)"
        let units = orderedUnits()
        if units.count > 1, backend.dualGlueExists() {
            r += "  \u{00B7}  Dual \(GlueRouting.dualInput(unitIndex: unitIndex, channel: c, channelsA: units[0].channels))"
            r += "  \u{00B7}  raw \(GlueRouting.rawCopyInput(unitIndex: unitIndex, channel: c, channelsA: units[0].channels, channelsB: units[1].channels))"
        }
        return r
    }

    private func buildCards(_ u: GlueUnit?) {
        cards.forEach { $0.removeFromSuperview() }
        cards = []
        empty.isHidden = u != nil
        globalCard.isHidden = u == nil
        guard let u = u else { layout(); return }
        for c in 0..<u.channels {
            let name = UserDefaults.standard.string(forKey: labelKey(u, c)) ?? (c < u.channelNames.count ? u.channelNames[c] : "Input \(c + 1)")
            let card = ChannelCard(index: c, label: name)
            card.rate = u.rate
            card.routing = routing(u, c)
            card.channel = settings.channels[c]
            card.onChange = { [weak self] ch in self?.channelChanged(c, ch) }
            card.onResetName = { [weak self, weak card] in
                guard let s = self, let u = s.currentUnit() else { return }
                UserDefaults.standard.removeObject(forKey: s.labelKey(u, c))
                card?.label = c < u.channelNames.count ? u.channelNames[c] : "Input \(c + 1)"
            }
            card.onRename = { [weak self, weak card] t in
                guard let s = self, let u = s.currentUnit() else { return }
                if t.isEmpty { UserDefaults.standard.removeObject(forKey: s.labelKey(u, c)); card?.label = c < u.channelNames.count ? u.channelNames[c] : "Input \(c + 1)" }
                else { UserDefaults.standard.set(t, forKey: s.labelKey(u, c)); card?.label = t }
            }
            matrix.addSubview(card)
            cards.append(card)
        }
        refreshLinks()
        for (k, bar) in stageBars.enumerated() { bar.selected = settings.stages[k] }
        drive.value = settings.drive
        updateDriveLabel()
        bypass.isOn = settings.bypass
        updateStatus()
        layout()
    }

    private func updateDriveLabel() { driveValue.stringValue = settings.drive == 0 ? "0 dB" : String(format: "+%.1f dB", settings.drive) }

    // MARK: stereo link
    private func channelChanged(_ c: Int, _ ch: GlueChannel) {
        let wasLinked = settings.channels[c].link
        settings.channels[c] = ch
        settings.propagateLink(from: c)
        if c + 1 < cards.count && (ch.link || wasLinked) { cards[c + 1].channel = settings.channels[c + 1] }
        refreshLinks()
        dirty = true
    }

    private func refreshLinks() {
        for (c, card) in cards.enumerated() { card.follows = settings.isFollower(c) ? c - 1 : nil }
    }

    // MARK: names (the Names menu)
    static let alsoSettingsKey = "templates.alsoSettings"
    var templatesAlsoSetSettings: Bool {
        get { return UserDefaults.standard.object(forKey: GlueWindowController.alsoSettingsKey) as? Bool ?? true }
        set { UserDefaults.standard.set(newValue, forKey: GlueWindowController.alsoSettingsKey) }
    }
    var hasTwoUnits: Bool { return backend.units.count >= 2 }

    private func applyTemplate(_ t: [GlueTemplateEntry], to u: GlueUnit) {
        var s = (u.key == currentUnit()?.key) ? settings : backend.settings(u)
        let names = GlueTemplates.apply(t, to: &s, channels: u.channels, settings: templatesAlsoSetSettings)
        for (c, n) in names.enumerated() { UserDefaults.standard.set(n, forKey: labelKey(u, c)) }
        if templatesAlsoSetSettings { backend.setSettings(u, s) }
        if u.key == currentUnit()?.key { settings = s }
    }

    func applyBand16() {
        guard let u = currentUnit() else { return }
        flush()
        applyTemplate(GlueTemplates.band16, to: u)
        buildCards(u)
    }

    func applyDual32() {
        let units = orderedUnits()
        guard units.count >= 2 else { return }
        flush()
        applyTemplate(GlueTemplates.dualA, to: units[0])
        applyTemplate(GlueTemplates.dualB, to: units[1])
        if let u = currentUnit() { buildCards(u) }
    }

    func resetNames() {
        guard let u = currentUnit() else { return }
        for c in 0..<u.channels { UserDefaults.standard.removeObject(forKey: labelKey(u, c)) }
        buildCards(u)
    }

    // MARK: layout
    func windowDidResize(_ notification: Notification) { layout() }

    private func layout() {
        let w = root.bounds.width, pad = Theme.pad
        title.frame = NSRect(x: pad, y: 16, width: 140, height: 20)
        if let bar = unitBar { bar.frame.origin = NSPoint(x: pad + 150, y: 11) }
        dualButton.isHidden = backend.units.count < 2
        dualButton.title = backend.dualGlueExists() ? "Remove Dual Glue" : "Create Dual Glue"
        dualButton.setFrameSize(dualButton.intrinsicContentSize)
        dualButton.frame.origin = NSPoint(x: w - pad - dualButton.frame.width, y: 13)
        bypass.setFrameSize(NSSize(width: 62, height: 22))
        bypass.frame.origin = NSPoint(x: (dualButton.isHidden ? w - pad : dualButton.frame.minX - 12) - 62, y: 15)
        let left = (unitBar?.frame.maxX ?? title.frame.maxX) + 16
        let sw = min(Theme.textWidth(status.stringValue, font: Theme.font(11)) + 8, max(0, bypass.frame.minX - 12 - left))
        status.frame = NSRect(x: bypass.frame.minX - 12 - sw, y: 18, width: sw, height: 16)
        status.alignment = .right
        let gy = Theme.headerHeight + pad
        globalCard.frame = NSRect(x: pad, y: gy, width: w - pad * 2, height: 120)
        // Two columns: Transformer / Desk on the left, Tube / Tape on the right, captions centred on their bars.
        let capW: CGFloat = 92
        let colW = capW + stageBars.reduce(0) { max($0, $1.frame.width) }
        let order = [0, 1, 2, 3]                                          // row 0: Transformer, Tube; row 1: Desk, Tape
        for k in order {
            let bar = stageBars[k]
            let row = k < 2 ? 0 : 1, col = k % 2
            let x = 18 + CGFloat(col) * (colW + 36)
            let y: CGFloat = Card.contentTop - 4 + CGFloat(row) * 38
            bar.frame.origin = NSPoint(x: x + capW, y: y)
            if let cap = globalCard.viewWithTag(100 + k) { cap.frame = NSRect(x: x, y: y + (bar.frame.height - 14) / 2 + 1, width: capW, height: 14) }
        }
        // Drive: right of the stage columns, caption and value on the first row, the slider on the second
        let dx = 18 + 2 * (colW + 36) - 12
        let dw = max(120, globalCard.frame.width - dx - 18)
        driveCaption.frame = NSRect(x: dx, y: Card.contentTop + 4, width: 60, height: 14)
        driveValue.frame = NSRect(x: dx + dw - 70, y: Card.contentTop + 2, width: 70, height: 16)
        drive.frame = NSRect(x: dx, y: Card.contentTop + 38 + 7, width: dw, height: 16)
        let my = gy + globalCard.frame.height + pad
        scroll.frame = NSRect(x: 0, y: my, width: w, height: max(0, root.bounds.height - my))
        empty.frame = NSRect(x: pad, y: my + 8, width: w - pad * 2, height: 18)
        let gap: CGFloat = 8
        let rows = (cards.count + GlueWindowController.perRow - 1) / GlueWindowController.perRow
        matrix.frame = NSRect(x: 0, y: 0, width: w, height: CGFloat(rows) * (ChannelCard.height + gap) + pad)
        for (i, card) in cards.enumerated() {
            let r = i / GlueWindowController.perRow, c = i % GlueWindowController.perRow
            card.frame.origin = NSPoint(x: pad + CGFloat(c) * (ChannelCard.width + gap), y: CGFloat(r) * (ChannelCard.height + gap))
        }
    }

    /// Fits the window to the matrix (up to the screen's height).
    func fitToContent() {
        let rows = (cards.count + GlueWindowController.perRow - 1) / GlueWindowController.perRow
        let want = Theme.headerHeight + Theme.pad + 120 + Theme.pad + CGFloat(max(rows, 1)) * (ChannelCard.height + 8) + Theme.pad
        let screenH = (window.screen ?? NSScreen.main)?.visibleFrame.height ?? 1000
        var f = window.frame
        let content = window.contentRect(forFrameRect: f)
        let h = min(want, screenH - (f.height - content.height))
        f.size.height = h + (f.height - content.height)
        window.setFrame(f, display: true)
        layout()
    }

    // MARK: state
    private func updateStatus() {
        guard let u = currentUnit() else { status.stringValue = ""; return }
        let m = backend.meters(u)
        let rate = u.rate.truncatingRemainder(dividingBy: 1000) == 0 ? String(format: "%.0f", u.rate / 1000) : String(format: "%.1f", u.rate / 1000)
        let text = "\(u.name)  \u{00B7}  \(m.running ? "running" : "idle")  \u{00B7}  \(rate) kHz  \u{00B7}  \(u.channels) inputs  \u{00B7}  latency \(u.latency)"
        if text != status.stringValue { status.stringValue = text; layout() }
    }

    private func updateMeters() {
        guard let u = currentUnit() else { return }
        let m = backend.meters(u)
        for (c, card) in cards.enumerated() {
            card.setMeters(gr: c < m.gr.count ? m.gr[c] : 0, hit: c < m.hit.count ? m.hit[c] : 0)
        }
        updateStatus()
    }

    private func flush() {
        guard dirty, let u = currentUnit() else { return }
        dirty = false
        backend.setSettings(u, settings)
    }

    private func toggleDual() {
        if backend.dualGlueExists() { backend.removeDualGlue() }
        else {
            let units = orderedUnits()
            guard units.count >= 2 else { return }
            let r = backend.createDualGlue(guidA: units[0].guidHex, guidB: units[1].guidHex)
            if r != noErr {
                let a = NSAlert(); a.messageText = "Could not create the Dual Glue aggregate"; a.informativeText = "Core Audio error \(r)."
                a.runModal()
            }
        }
        for (c, card) in cards.enumerated() { if let u = currentUnit() { card.routing = routing(u, c) } }
        layout()
    }

    func debugBeginRename(_ c: Int) { if c < cards.count { cards[c].beginRename() } }

    /// --snapshot: show another unit, optionally with the Dual Glue aggregate created (demo backend only changes memory).
    func selectForSnapshot(unit i: Int, dual: Bool = false) {
        if dual && !backend.dualGlueExists() { toggleDual() }
        selectUnit(min(i, max(0, backend.units.count - 1)))
    }

    func windowWillClose(_ notification: Notification) { flush(); timers.forEach { $0.invalidate() }; NSApp.terminate(nil) }
}
