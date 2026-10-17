// PN.3 -- one controller per interface (the reference keeps one CXDevice and its windows per device, CDeviceWindowManager): header band
// (device name with a device switcher when several interfaces are connected, the rate, the four tabs, the view switch full / wide meters /
// narrow meters), the tab content, the fixed sidebar. Drives the model callbacks, the ~30 Hz meter poll (reference LPeriodical, A2-P9)
// and the two mini-meter windows. Only one of the three windows is visible at a time, as in the reference; the one shown is centred where
// the previous one was (the screen centre the first time). The full window fits itself to the selected tab.
import AppKit

/// Header band: device name (a menu of the connected interfaces when there are several) with the current rate, the tab bar in the
/// middle, the view switch on the right.
final class HeaderView: FlippedView {
    let tabs = SegmentedBar(items: [.text("Mixer"), .text("Output"), .text("Hardware"), .text("About")], segmentWidth: 92)
    let modes = SegmentedBar(items: [.icon(.full), .icon(.wide), .icon(.narrow)], segmentWidth: 34,
                             tooltips: ["Full panel", "Wide meters", "Narrow meters"])
    var name = "" { didSet { relayout() } }
    var rate = "" { didSet { relayout() } }
    var locked = true { didSet { needsDisplay = true } }
    /// Connected interfaces (id, menu title); the name becomes a menu when there is more than one.
    var devices: [(id: UInt64, title: String)] = [] { didSet { relayout() } }
    var currentID: UInt64 = 0
    var onSelectDevice: ((UInt64) -> Void)?
    private var nameRect = NSRect.zero
    private let chooserTip: NSString = "Switch to another connected FireWire 1814"     // kept: addToolTip does not retain its owner

    static let nameFont = Theme.font(14, .semibold)
    private var chooser: Bool { return devices.count > 1 }

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        addSubview(tabs); addSubview(modes)
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    /// Width of the name, switcher chevron and rate chip.
    var leftWidth: CGFloat {
        return Theme.pad + 4 + Theme.textWidth(name, font: HeaderView.nameFont) + (chooser ? 28 : 8) + 14 + 13 + Theme.textWidth(rate, font: Theme.mono(12, .medium))
    }

    private func relayout() {
        let w = bounds.width, y = ((bounds.height - 30) / 2).rounded()
        // tabs centred in the window, pushed right if the name would run into them
        tabs.setFrameOrigin(NSPoint(x: max(((w - tabs.frame.width) / 2).rounded(), leftWidth + 24), y: y))
        modes.setFrameOrigin(NSPoint(x: w - Theme.pad - modes.frame.width, y: y))
        let nw = Theme.textWidth(name, font: HeaderView.nameFont)
        nameRect = NSRect(x: Theme.pad - 4, y: y, width: nw + (chooser ? 32 : 16), height: 30)
        toolTip = nil
        removeAllToolTips()
        if chooser { addToolTip(nameRect, owner: chooserTip, userData: nil) }
        needsDisplay = true
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        relayout()
    }

    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) {
        Theme.chrome.setFill(); dirtyRect.fill()
        if chooser { Theme.fill(nameRect, radius: 8, Theme.raised) }
        let nw = Theme.textWidth(name, font: HeaderView.nameFont)
        Theme.draw(name, in: NSRect(x: Theme.pad + 4, y: 0, width: nw + 4, height: bounds.height), font: HeaderView.nameFont, color: Theme.text, align: .left)
        var x = Theme.pad + 4 + nw + 8
        if chooser {                                            // chevron: this name is a device menu
            let c = NSPoint(x: x + 6, y: bounds.midY)
            let p = NSBezierPath()
            p.move(to: NSPoint(x: c.x - 4, y: c.y - 2)); p.line(to: NSPoint(x: c.x, y: c.y + 2)); p.line(to: NSPoint(x: c.x + 4, y: c.y - 2))
            p.lineWidth = 1.6; p.lineCapStyle = .roundLineCapStyle; p.lineJoinStyle = .roundLineJoinStyle
            Theme.text2.setStroke(); p.stroke()
            x += 20
        }
        x += 14
        let cy = bounds.midY
        (locked ? Theme.green : Theme.red).setFill()
        NSBezierPath(ovalIn: NSRect(x: x, y: cy - 3.5, width: 7, height: 7)).fill()
        Theme.draw(rate, in: NSRect(x: x + 13, y: 0, width: 120, height: bounds.height), font: Theme.mono(12, .medium), color: Theme.text2, align: .left)
    }

    override func mouseDown(with event: NSEvent) {
        let p = convert(event.locationInWindow, from: nil)
        guard chooser, nameRect.contains(p) else { return }
        let menu = NSMenu()
        for d in devices {
            let item = NSMenuItem(title: d.title, action: #selector(pick(_:)), keyEquivalent: "")
            item.target = self
            item.representedObject = NSNumber(value: d.id)
            item.state = d.id == currentID ? .on : .off
            menu.addItem(item)
        }
        menu.popUp(positioning: nil, at: NSPoint(x: nameRect.minX, y: nameRect.maxY + 4), in: self)
    }
    @objc private func pick(_ item: NSMenuItem) {
        if let n = item.representedObject as? NSNumber, n.uint64Value != currentID { onSelectDevice?(n.uint64Value) }
    }
}

/// Window content: header on top, tab content on the left, sidebar on the right.
final class RootView: FlippedView {
    let header: HeaderView
    let body: NSView
    let sidebar: NSView
    init(header: HeaderView, body: NSView, sidebar: NSView) {
        self.header = header; self.body = body; self.sidebar = sidebar
        super.init(frame: .zero)
        [header, body, sidebar].forEach { addSubview($0) }
    }
    required init?(coder: NSCoder) { fatalError("unused") }
    override var isOpaque: Bool { return true }
    override func draw(_ dirtyRect: NSRect) { Theme.window.setFill(); dirtyRect.fill() }
    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        let h = Theme.headerHeight, sw = Theme.sidebarWidth
        header.frame = NSRect(x: 0, y: 0, width: newSize.width, height: h)
        sidebar.frame = NSRect(x: newSize.width - sw, y: h, width: sw, height: max(0, newSize.height - h))
        body.frame = NSRect(x: 0, y: h, width: max(0, newSize.width - sw), height: max(0, newSize.height - h))
    }
}

final class PanelWindowController: NSObject, NSWindowDelegate {
    var device: PanelBackend
    let ctx: PanelContext
    let window: NSWindow
    private var tabs: [(name: String, view: NSView & PanelTab)] = []
    private var sidebar: SidebarView!
    private let presets: PresetStore
    private let tabView = NSTabView()
    private let header = HeaderView(frame: .zero)
    private var wide: MiniMetersWindow?, narrow: MiniMetersWindow?
    private(set) var mode = -1                          // -1 = never shown, 0 full, 1 wide, 2 narrow
    private var fittedFor: NSSize?
    private var meterTimer: Timer?
    var onSelectDevice: ((PanelWindowController, UInt64) -> Void)?

    /// Content size before the first fit (the 48 kHz ADAT mixer).
    static let defaultContentSize = NSSize(width: 1290, height: 680)
    /// Narrowest window that keeps the header readable (name, tabs, view switch); the height always fits the sidebar.
    private var minContentWidth: CGFloat {
        return ceil(header.leftWidth + 24 + header.tabs.frame.width + 24 + header.modes.frame.width + Theme.pad)
    }

    init(device: PanelBackend) {
        self.device = device
        let model = DeviceModel(transport: device, rate: device.rate)
        // Headphone safety (clone addition): phone levels above the ceiling are lowered on open; persistence is the helper's job (A4 H11)
        if !(device is DemoBackend) { model.limitPhonesLevelsAtStartup() }
        ctx = PanelContext(model: model, device: device)
        presets = PresetStore(model: model)
        window = NSWindow(contentRect: NSRect(origin: .zero, size: PanelWindowController.defaultContentSize),
                          styleMask: [.titled, .closable, .miniaturizable, .resizable], backing: .buffered, defer: false)
        super.init()
        window.title = "M-Audio FireWire 1814"
        window.isReleasedWhenClosed = false
        window.delegate = self
        styleWindow(window)
        build()

        device.onPropertiesChanged = { [weak self] in self?.ctx.model.refresh() }
        model.onKernelChange = { [weak self] _ in self?.refreshAll() }
        model.onLocalChange = { [weak self] _ in self?.refreshAll() }
        meterTimer = Timer.scheduledTimer(withTimeInterval: 1.0 / 30.0, repeats: true) { [weak self] _ in self?.tickMeters() }
        RunLoop.main.add(meterTimer!, forMode: .commonModes)           // meters keep moving while scrolling or in a menu
    }

    private func build() {
        tabs = [("mixer", MixerTab(context: ctx)), ("output", OutputTab(context: ctx)),
                ("hardware", HardwareTab(context: ctx)), ("about", AboutTab(context: ctx))]
        tabView.tabViewType = .noTabsNoBorder
        tabView.drawsBackground = false
        for (name, v) in tabs {
            let item = NSTabViewItem(identifier: name)
            item.label = name
            item.view = v
            tabView.addTabViewItem(item)
        }
        sidebar = SidebarView(context: ctx, presets: presets, window: window)
        header.name = device.info.name
        header.currentID = device.entryID
        header.tabs.onSelect = { [weak self] i in
            guard let me = self else { return }
            me.selectTab(me.tabs[i].name)
        }
        header.modes.onSelect = { [weak self] i in self?.setMode(i) }
        header.onSelectDevice = { [weak self] id in
            guard let me = self else { return }
            me.onSelectDevice?(me, id)
        }
        let root = RootView(header: header, body: tabView, sidebar: sidebar)
        window.contentView = root
        refreshHeader()
    }

    // MARK: interfaces

    /// Name shown in the header and the meter windows ("FireWire 1814", plus the GUID tail when several are connected).
    var displayName: String = "" {
        didSet { header.name = displayName; wide?.deviceTitle = displayName; narrow?.deviceTitle = displayName }
    }
    func setDevices(_ list: [(id: UInt64, title: String)]) { header.devices = list }

    /// The visible window of this interface (full panel or a meter window), if any.
    var visibleWindow: NSWindow? { return [window, wide?.window, narrow?.window].compactMap { $0 }.first(where: { $0.isVisible }) }
    func owns(_ w: NSWindow?) -> Bool { return w != nil && (w === window || w === wide?.window || w === narrow?.window) }
    func hideAll() { [window, wide?.window, narrow?.window].compactMap { $0 }.forEach { $0.orderOut(nil) } }

    func show() { setMode(mode < 0 ? 0 : mode) }

    // MARK: view modes and placement

    /// 0 = full window, 1 = wide meters, 2 = narrow meters. The target window takes the centre of the window it replaces (or `center`).
    func setMode(_ m: Int, center: NSPoint? = nil) {
        header.modes.selected = m
        let c = center ?? visibleWindow.map { NSPoint(x: $0.frame.midX, y: $0.frame.midY) } ?? screenCenter()
        let target: NSWindow
        switch m {
        case 1:
            if wide == nil { wide = MiniMetersWindow(context: ctx, narrow: false, onSwitch: { [weak self] in self?.setMode($0) }); wide?.deviceTitle = displayName }
            wide!.update(); target = wide!.window
        case 2:
            if narrow == nil { narrow = MiniMetersWindow(context: ctx, narrow: true, onSwitch: { [weak self] in self?.setMode($0) }); narrow?.deviceTitle = displayName }
            narrow!.update(); target = narrow!.window
        default:
            target = window
            if mode == -1 { fitToTab(animate: false) }
        }
        if mode != m || !target.isVisible || center != nil { placeWindow(target, center: c) }
        target.makeKeyAndOrderFront(nil)
        for w in [window, wide?.window, narrow?.window].compactMap({ $0 }) where w !== target { w.orderOut(nil) }
        mode = m
    }

    /// Resizes the full window to the selected tab: width = tab + sidebar, height = the taller of tab and sidebar; the top edge and the
    /// horizontal centre stay, and the window stays on its screen.
    private func fitToTab(animate: Bool) {
        guard let t = currentTab else { return }
        let pref = t.preferredSize
        fittedFor = pref
        let minSize = NSSize(width: minContentWidth, height: Theme.headerHeight + sidebar.preferredSize.height)
        window.contentMinSize = minSize
        var content = NSSize(width: max(minSize.width, ceil(pref.width) + Theme.sidebarWidth), height: max(minSize.height, Theme.headerHeight + ceil(pref.height)))
        let screen = window.screen ?? NSScreen.main ?? NSScreen.screens.first
        if let vf = screen?.visibleFrame {
            let chrome = window.frameRect(forContentRect: NSRect(origin: .zero, size: content)).size
            content.width -= max(0, chrome.width - vf.width)
            content.height -= max(0, chrome.height - vf.height)
        }
        let old = window.frame
        var f = window.frameRect(forContentRect: NSRect(origin: .zero, size: content))
        f.origin = NSPoint(x: (old.midX - f.width / 2).rounded(), y: old.maxY - f.height)
        if let vf = screen?.visibleFrame {
            f.origin.x = max(vf.minX, min(f.origin.x, vf.maxX - f.width))
            f.origin.y = max(vf.minY, min(f.origin.y, vf.maxY - f.height))
        }
        if f != old { window.setFrame(f, display: true, animate: animate && window.isVisible) }
    }

    private var currentTab: (NSView & PanelTab)? {
        guard let sel = tabView.selectedTabViewItem else { return nil }
        return tabs.first(where: { $0.name == (sel.identifier as? String) })?.view
    }

    // MARK: model -> views

    private func refreshAll() {
        for (_, v) in tabs { v.refresh() }
        sidebar.refresh()
        refreshHeader()
        // a layout change (rate, optical ports) can change the selected tab's size: refit then
        if mode >= 0, let t = currentTab, t.preferredSize != fittedFor { fitToTab(animate: true) }
    }

    private func refreshHeader() {
        let m = ctx.model
        header.rate = "\(Theme.rateText(m.sampleRate)) kHz"
        header.locked = m.rateLocked
    }

    private func tickMeters() {
        ctx.pollMeters()
        if window.isVisible { currentTab?.updateMeters() }
        if let w = wide, w.window.isVisible { w.update() }
        if let n = narrow, n.window.isVisible { n.update() }
    }

    func selectTab(_ name: String) {
        guard let i = tabs.index(where: { $0.name == name }) else { return }
        tabView.selectTabViewItem(withIdentifier: name)
        header.tabs.selected = i
        tabs[i].view.updateMeters()
        if mode >= 0 { fitToTab(animate: true) }
    }
    func miniWindow(narrow isNarrow: Bool) -> NSWindow? { return (isNarrow ? narrow : wide)?.window }
    func turnLevelKnob(_ n: Int) { sidebar.turnLevelKnob(n) }
    func hardwareTab() -> HardwareTab? { return tabs.first(where: { $0.name == "hardware" })?.view as? HardwareTab }

    func close() {
        meterTimer?.invalidate(); meterTimer = nil
        device.onPropertiesChanged = nil
        ctx.model.onKernelChange = nil; ctx.model.onLocalChange = nil
        hideAll()
    }

    /// Closing the panel hides it; the interface keeps its controller and comes back from the Device menu or the Dock.
    func windowShouldClose(_ sender: NSWindow) -> Bool { hideAll(); return false }
}
