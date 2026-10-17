// PN.5 -- the hardware tab (wireframe PN1, screenshots P12): sample rate with the sync indicator, sync source, digital port selection, and
// the a/b switch monitoring settings with the headphone A/B source sheet. Every digital-port widget is a read-modify-write of one bit of
// MAClockSource (P-ClockUI-encoding §2, MAClockSource.wireValue keeps the gate). Greying follows ChannelLayout (device truth, P12).
// No buffer-size control: on the Mac CoreAudio sets it in the host (user, 2026-10-15). Layout: a 2 x 2 grid of equal-width cards.
import AppKit

/// Short name of a headphone source mask as the a/b text shows it (MAHeadphoneSource: 1 = out 1/2, 2 = out 3/4, 4 = aux).
func headphoneSourceName(_ mask: Int) -> String {
    switch mask {
    case 1: return "out 1/2"
    case 2: return "out 3/4"
    case 4: return "aux"
    default: return "-"
    }
}

final class HardwareTab: FlippedView, PanelTab {
    private let ctx: PanelContext
    private let rateCard = Card(title: "Sample Rate"), syncCard = Card(title: "Sync Source")
    private let portsCard = Card(title: "Digital Ports"), monCard = Card(title: "A/B Switch")
    private let rateDisplay = RateDisplay()
    private let rateStatus = makeLabel("", size: 11, weight: .medium)
    private let stepper = StepperControl()
    private var syncRadios: [(ChoiceButton, MAClockSource.Sync)] = []
    private let inAdat = ChoiceButton(radio: "ADAT"), inSpdif = ChoiceButton(radio: "S/PDIF")
    private let outAdat = ChoiceButton(radio: "ADAT"), outSpdif = ChoiceButton(radio: "S/PDIF")
    private let portOptical = ChoiceButton(radio: "Optical"), portCoax = ChoiceButton(radio: "Coaxial")
    private let smux = ChoiceButton(check: "ADAT S/MUX")
    private var portLabels: [NSTextField] = []
    private let dmRadio = ChoiceButton(radio: "Direct monitoring")
    private let hpRadio = ChoiceButton(radio: "Headphone out 1 A/B")
    private let dmNote = makeLabel("A = DM off, LED off\nB = DM on, LED on", size: 11, color: Theme.text3)
    private let hpNote = makeLabel("", size: 11, color: Theme.text3)
    private let setSource = FlatButton("Set Source...")
    private var sheet: HPABSheet?

    private var rates: [UInt32] { return ctx.model.supportedRates }   // CoreAudio's list for the device (A2-P8 §10); the kext's six rates until the HAL device exists

    init(context: PanelContext) {
        ctx = context
        super.init(frame: .zero)
        build()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func build() {
        let m = ctx.model
        [rateCard, syncCard, portsCard, monCard].forEach { addSubview($0) }

        // sample rate: stepper through the six rates, display green/closed lock when synced, red/open lock otherwise (user, P12 item 8)
        stepper.onStep = { [weak self] dir in self?.stepRate(dir) }
        rateCard.addSubview(stepper); rateCard.addSubview(rateDisplay); rateCard.addSubview(rateStatus)

        // sync source: bits [1:0] (v1.9 capture; v1.5.2 values unverified)
        let syncDefs: [(String, MAClockSource.Sync)] = [("Internal - digital inputs muted", .internalMuted), ("Internal - digital inputs unmuted", .internalUnmuted),
                                                        ("External - digital", .externalDigital), ("Word clock", .wordClock)]
        for (t, s) in syncDefs {
            let b = ChoiceButton(radio: t)
            b.onAction = { [weak self] in
                m.updateClock { $0.sync = s }
                self?.refresh()
            }
            syncRadios.append((b, s)); syncCard.addSubview(b)
        }

        // digital ports
        inAdat.onAction = { [weak self] in m.updateClock { $0.adatIn = true }; self?.afterClockWrite() }
        inSpdif.onAction = { [weak self] in m.updateClock { $0.adatIn = false }; self?.afterClockWrite() }
        outAdat.onAction = { [weak self] in m.updateClock { $0.adatOut = true }; self?.afterClockWrite() }
        outSpdif.onAction = { [weak self] in m.updateClock { $0.adatOut = false }; self?.afterClockWrite() }
        portOptical.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = false }; self?.refresh() }
        portCoax.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = true }; self?.refresh() }
        smux.onAction = { [weak self, weak smux] in
            let on = smux?.on ?? false
            m.updateClock { $0.smux = on }
            self?.refresh()
        }
        portLabels = ["Input", "Output", "S/PDIF input"].map { makeLabel($0, size: 12, color: Theme.text3) }
        (portLabels as [NSView] + [inAdat, inSpdif, outAdat, outSpdif, portOptical, portCoax, smux]).forEach { portsCard.addSubview($0) }

        // a/b switch function: cookie 0x608 value 3 -> MAMomentarySwitchSource 1 (direct monitoring), else headphone A/B (P13 §5)
        dmRadio.onAction = { [weak self] in m.setSwitchFunction(directMonitoring: true); self?.refresh() }
        hpRadio.onAction = { [weak self] in m.setSwitchFunction(directMonitoring: false); self?.refresh() }
        setSource.onAction = { [weak self] in self?.showSheet() }
        dmNote.maximumNumberOfLines = 2; hpNote.maximumNumberOfLines = 2
        [dmRadio, dmNote, hpRadio, hpNote, setSource].forEach { monCard.addSubview($0) }
        layoutCards()
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        layoutCards()
    }

    /// 2 x 2 grid of equal cards; card width follows the tab width between 300 and 420 pt.
    private func layoutCards() {
        let pad = Theme.pad, gap: CGFloat = 16
        let w = max(300, min(420, ((bounds.width - 2 * pad - gap) / 2).rounded(.down)))
        let row1: CGFloat = 186, row2: CGFloat = 228
        rateCard.frame = NSRect(x: pad, y: pad, width: w, height: row1)
        syncCard.frame = NSRect(x: pad + w + gap, y: pad, width: w, height: row1)
        portsCard.frame = NSRect(x: pad, y: pad + row1 + gap, width: w, height: row2)
        monCard.frame = NSRect(x: pad + w + gap, y: pad + row1 + gap, width: w, height: row2)
        let inner: CGFloat = 18, top = Card.contentTop

        // rate: stepper | display, status line under it
        stepper.frame = NSRect(x: inner, y: top + 6, width: 26, height: 76)
        rateDisplay.frame = NSRect(x: inner + 36, y: top + 6, width: w - 2 * inner - 36, height: 76)
        rateStatus.frame = NSRect(x: inner + 36, y: top + 96, width: w - 2 * inner - 36, height: 18)

        for (i, (b, _)) in syncRadios.enumerated() { b.frame = NSRect(x: inner, y: top + 6 + CGFloat(i) * 32, width: w - 2 * inner, height: 20) }

        let labelW: CGFloat = 112, col1 = inner + labelW, col2 = col1 + 96
        let rowsY: [CGFloat] = [top + 6, top + 42, top + 78]
        for (i, l) in portLabels.enumerated() { l.frame = NSRect(x: inner, y: rowsY[i] + 1, width: labelW - 8, height: 18) }
        for (i, pair) in [(inAdat, inSpdif), (outAdat, outSpdif), (portOptical, portCoax)].enumerated() {
            pair.0.frame = NSRect(x: col1, y: rowsY[i], width: 92, height: 20)
            pair.1.frame = NSRect(x: col2, y: rowsY[i], width: 92, height: 20)
        }
        smux.frame = NSRect(x: col1, y: top + 126, width: 160, height: 20)

        dmRadio.frame = NSRect(x: inner, y: top + 6, width: w - 2 * inner, height: 20)
        dmNote.frame = NSRect(x: inner + 24, y: top + 30, width: w - 2 * inner - 24, height: 32)
        hpRadio.frame = NSRect(x: inner, y: top + 76, width: w - 2 * inner, height: 20)
        hpNote.frame = NSRect(x: inner + 24, y: top + 100, width: w - 2 * inner - 24, height: 32)
        setSource.frame = NSRect(x: inner + 24, y: top + 140, width: setSource.intrinsicContentSize.width, height: 26)
    }

    private func stepRate(_ dir: Int) {
        let cur = rates.index(of: ctx.model.sampleRate) ?? 0
        let i = cur + dir
        if i >= 0 && i < rates.count { ctx.model.requestSampleRate(rates[i]) }
        refresh()
    }

    /// Optical in/out changed: S/MUX follows the device rule (forced on at 88.2/96 kHz with ADAT), then redraw everything.
    private func afterClockWrite() {
        let l = ctx.layout
        if l.smuxWanted != ctx.model.clockSource.smux { ctx.model.updateClock { $0.smux = l.smuxWanted } }
        refresh()
    }

    func rebuild() { refresh() }
    /// Two 380-pt cards per row, two rows.
    var preferredSize: NSSize { return NSSize(width: 2 * Theme.pad + 2 * 380 + 16, height: 2 * Theme.pad + 186 + 16 + 228) }

    func refresh() {
        let m = ctx.model
        let c = m.clockSource
        let l = ctx.layout
        rateDisplay.text = Theme.rateText(m.sampleRate)
        let locked = m.rateLocked
        rateDisplay.locked = locked
        rateStatus.stringValue = locked ? "Locked" : "Not locked"
        rateStatus.textColor = locked ? Theme.green : Theme.red
        let i = rates.index(of: m.sampleRate)
        stepper.canUp = (i ?? 0) < rates.count - 1
        stepper.canDown = (i ?? 0) > 0
        for (b, s) in syncRadios {
            b.on = (c.sync == s)
            b.isEnabled = (s == .externalDigital) ? l.externalDigitalSyncEnabled : true
        }
        inAdat.on = c.adatIn; inSpdif.on = !c.adatIn
        outAdat.on = c.adatOut; outSpdif.on = !c.adatOut
        for b in [inAdat, inSpdif, outAdat, outSpdif] { b.isEnabled = l.portCardEnabled }
        portCoax.on = c.spdifCoaxial; portOptical.on = !c.spdifCoaxial
        portOptical.isEnabled = l.spdifPortEnabled; portCoax.isEnabled = l.spdifPortEnabled
        smux.on = l.smuxShownChecked
        smux.isEnabled = l.smuxEnabled
        portLabels[0].textColor = l.portCardEnabled ? Theme.text3 : Theme.text4
        portLabels[1].textColor = l.portCardEnabled ? Theme.text3 : Theme.text4
        portLabels[2].textColor = l.spdifPortEnabled ? Theme.text3 : Theme.text4
        let dm = (m.number(MAKeys.momentarySwitchSource) ?? 0) != 0
        dmRadio.on = dm; hpRadio.on = !dm
        setSource.isEnabled = !dm
        // A/B text follows the sources set in the sheet: MAMomentarySwitchRoutings entry 0 = A, entry 1 = B. Correction (2026-10-16, run on
        // the real device): entry index = the kext's A/B mode (fMode 0 = A, 1 = B; SetMomentarySwitchRoutingSource idx-1 == fMode), and the
        // kext default {1, 2, 3, 4} then reads A = out 1/2, B = out 3/4, the panel's documented default. The earlier 1/2 reading showed "3/4, -".
        let routes = m.ints(MAKeys.momentarySwitchRoutings)
        let a = routes.count > 0 ? routes[0] : 0, b = routes.count > 1 ? routes[1] : 0
        hpNote.stringValue = "A = \(headphoneSourceName(a)), LED off\nB = \(headphoneSourceName(b)), LED on"
    }
    func updateMeters() {}

    /// Debug/demo aid (snapshot): opens the A/B sheet and returns its window.
    func presentABSheet() -> NSWindow? { showSheet(); return sheet?.panel }

    // MARK: headphone A/B sheet (P11 layout; A2-P13 §5 write)
    private func showSheet() {
        guard let window = self.window else { return }
        let routes = ctx.model.ints(MAKeys.momentarySwitchRoutings)
        // An entry that is not a source mask (1, 2, 4) starts with no choice, as the a/b text shows "-" for it.
        func source(_ i: Int) -> MAHeadphoneSource? { return routes.count > i && routes[i] > 0 ? MAHeadphoneSource(rawValue: UInt32(routes[i])) : nil }
        let s = HPABSheet(a: source(0), b: source(1))
        sheet = s
        s.onSet = { [weak self] a, b in self?.ctx.model.setABSources(a: a, b: b); self?.refresh() }
        window.beginSheet(s.panel) { [weak self] _ in self?.sheet = nil }
    }
}

/// "Headphone Out 1 A/B Source Settings": one source of three for A and for B; the source held by the other side is disabled; Set
/// stays disabled until a choice changes (screenshots P12) and both sides have a source.
final class HPABSheet {
    let panel = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 440, height: 214), styleMask: [.titled], backing: .buffered, defer: false)
    private var a: MAHeadphoneSource?, b: MAHeadphoneSource?
    private let original: (MAHeadphoneSource?, MAHeadphoneSource?)
    private var aRadios: [ChoiceButton] = [], bRadios: [ChoiceButton] = []
    private let setButton = FlatButton("Set")
    var onSet: ((MAHeadphoneSource, MAHeadphoneSource) -> Void)?
    private let all: [(String, MAHeadphoneSource)] = [("Out 1/2", .out12), ("Out 3/4", .out34), ("Aux", .aux)]

    init(a: MAHeadphoneSource?, b: MAHeadphoneSource?) {
        self.a = a; self.b = b; original = (a, b)
        panel.title = "Headphone Out 1 A/B Source Settings"
        styleWindow(panel)
        let root = FillView(Theme.surface)
        root.frame = NSRect(x: 0, y: 0, width: 440, height: 214)
        let heading = makeLabel("Headphone Out 1 A/B Source", size: 14, weight: .semibold, color: Theme.text)
        heading.frame = NSRect(x: 24, y: 20, width: 392, height: 20)
        let text = makeLabel("Select the source for the A and B positions of the Headphone Out 1 A/B switch.", size: 12, color: Theme.text2)
        text.maximumNumberOfLines = 2; text.lineBreakMode = .byWordWrapping
        text.frame = NSRect(x: 24, y: 46, width: 392, height: 34)
        [heading, text].forEach { root.addSubview($0) }
        func row(_ name: String, y: CGFloat, side: Int) -> [ChoiceButton] {
            let l = makeLabel(name, size: 12, color: Theme.text3); l.frame = NSRect(x: 24, y: y + 1, width: 80, height: 18); root.addSubview(l)
            var radios: [ChoiceButton] = []
            for (i, (t, s)) in all.enumerated() {
                let r = ChoiceButton(radio: t)
                r.frame = NSRect(x: 112 + CGFloat(i) * 100, y: y, width: 92, height: 20)
                r.onAction = { [weak self] in
                    guard let me = self else { return }
                    if side == 0 { me.a = s } else { me.b = s }
                    me.sync()
                }
                radios.append(r); root.addSubview(r)
            }
            return radios
        }
        aRadios = row("A source", y: 94, side: 0)
        bRadios = row("B source", y: 126, side: 1)
        let cancel = FlatButton("Cancel")
        cancel.frame = NSRect(x: 440 - 24 - 84 - 10 - 84, y: 166, width: 84, height: 26)
        setButton.frame = NSRect(x: 440 - 24 - 84, y: 166, width: 84, height: 26)
        setButton.primary = true
        setButton.keyEquivalent = "\r"
        cancel.keyEquivalent = "\u{1b}"
        cancel.onAction = { [weak self] in self.map { $0.panel.sheetParent?.endSheet($0.panel) } }
        setButton.onAction = { [weak self] in
            guard let me = self, let a = me.a, let b = me.b else { return }
            me.onSet?(a, b)
            me.panel.sheetParent?.endSheet(me.panel)
        }
        [cancel, setButton].forEach { root.addSubview($0) }
        panel.contentView = root
        sync()
    }

    private func sync() {
        for (i, (_, s)) in all.enumerated() {
            aRadios[i].on = (a == s); bRadios[i].on = (b == s)
            // The source held by the other side is not selectable (all three sources, incl. Aux: screenshots 11.39.31 / 11.39.49 PM).
            aRadios[i].isEnabled = !(b == s)
            bRadios[i].isEnabled = !(a == s)
        }
        setButton.isEnabled = a != nil && b != nil && (a != original.0 || b != original.1)
    }
}
