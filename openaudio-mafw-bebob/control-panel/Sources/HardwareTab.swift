// PN.5 / AP-5.1 -- the hardware tab (wireframe PN1, screenshots P12): sample rate with the sync indicator, the sync sources of the model's
// profile, the model's own card (the FW1814's digital ports, Models/FW1814/FW1814PortsCard.swift), and the a/b switch with the headphone A/B
// source sheet over the profile's sources. Clock writes are read-modify-writes of MAClockSource (P-ClockUI-encoding; wireValue keeps the gate).
// No buffer-size control: on the Mac CoreAudio sets it in the host (user, 2026-10-15). Layout: a 2 x 2 grid of equal-width cards.
import AppKit


final class HardwareTab: FlippedView, PanelTab {
    private let ctx: PanelContext
    private let rateCard = Card(title: "Sample Rate"), syncCard = Card(title: "Sync Source")
    private let monCard = Card(title: "A/B Switch")
    // Phones knob step (plan v7 AP-5.4, clone key MAMonitorKnobStep): the phones encoder, and the Level Controller on phones levels; outputs
    // and aux out follow the official knob law whatever this says (R127)
    private let knobCard = Card(title: "Phones Knob")
    private let knobRadios: [(ChoiceButton, UInt64)] = [(ChoiceButton(radio: "1.5 dB per click"), 3), (ChoiceButton(radio: "3 dB per click"), 6),
                                                         (ChoiceButton(radio: "Official law"), 0)]
    private var modelCard: HardwareCard?                    // the model's own card (FW1814: digital ports), in the bottom-left cell
    private let rateDisplay = RateDisplay()
    private let rateStatus = makeLabel("", size: 11, weight: .medium)
    private let stepper = StepperControl()
    // RATE_FOLLOW_PLAN_v1 (user, 2026-10-22): follow the rate of an external digital source; greyed with a driver that does not publish it
    private let followCheck = ChoiceButton(check: "Follow external clock rate")
    private var syncRadios: [(ChoiceButton, MAClockSource.Sync)] = []
    private let dmRadio = ChoiceButton(radio: "Direct monitoring")
    private let hpRadio: ChoiceButton
    private let dmNote = makeLabel("A = DM off, LED off\nB = DM on, LED on", size: 11, color: Theme.text3)
    private let hpNote = makeLabel("", size: 11, color: Theme.text3)
    private let setSource = FlatButton("Set Source...")
    private var sheet: HPABSheet?

    private var rates: [UInt32] { return ctx.model.supportedRates }   // CoreAudio's list for the device (A2-P8 §10); the kext's six rates until the HAL device exists

    init(context: PanelContext) {
        ctx = context
        hpRadio = ChoiceButton(radio: context.profile.hardware.abSwitchTitle)
        super.init(frame: .zero)
        build()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func build() {
        let m = ctx.model
        [rateCard, syncCard, monCard].forEach { addSubview($0) }
        if let c = ctx.profile.hardwareCard(ctx) {
            c.onChange = { [weak self] in self?.refresh() }
            modelCard = c; addSubview(c.card)
        }

        // sample rate: stepper through the six rates, display green/closed lock when synced, red/open lock otherwise (user, P12 item 8)
        stepper.onStep = { [weak self] dir in self?.stepRate(dir) }
        rateCard.addSubview(stepper); rateCard.addSubview(rateDisplay); rateCard.addSubview(rateStatus)
        followCheck.onAction = { [weak self] in PanelPrefs.followExternalRate = !PanelPrefs.followExternalRate; self?.refresh() }
        rateCard.addSubview(followCheck)

        // sync source: MAClockSource bits [1:0], the options the model offers (FW1814: v1.9 capture; Audiophile: internal / S/PDIF, PNAP §7)
        for opt in ctx.profile.hardware.sync {
            let s = opt.sync
            let b = ChoiceButton(radio: opt.title)
            b.onAction = { [weak self] in
                m.updateClock { $0.sync = s }
                self?.refresh()
            }
            syncRadios.append((b, s)); syncCard.addSubview(b)
        }

        // a/b switch function: cookie 0x608 value 3 -> MAMomentarySwitchSource 1 (direct monitoring), else headphone A/B (P13 §5)
        dmRadio.onAction = { [weak self] in m.setSwitchFunction(directMonitoring: true); self?.refresh() }
        hpRadio.onAction = { [weak self] in m.setSwitchFunction(directMonitoring: false); self?.refresh() }
        setSource.onAction = { [weak self] in self?.showSheet() }
        dmNote.maximumNumberOfLines = 2; hpNote.maximumNumberOfLines = 2
        [dmRadio, dmNote, hpRadio, hpNote, setSource].forEach { monCard.addSubview($0) }
        addSubview(knobCard)
        for (b, v) in knobRadios {
            b.onAction = { [weak self] in m.setNumber(MAKeys.monitorKnobStep, v); m.commit(); self?.refresh() }
            knobCard.addSubview(b)
        }
        layoutCards()
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        layoutCards()
    }

    /// 2 x 2 grid of equal cards (rate, sync / model card, a/b switch); card width follows the tab width between 300 and 420 pt. Without a
    /// model card the a/b switch card takes the bottom-left cell.
    private func layoutCards() {
        let pad = Theme.pad, gap: CGFloat = 16
        let w = max(300, min(420, ((bounds.width - 2 * pad - gap) / 2).rounded(.down)))
        let row1: CGFloat = 186, row2: CGFloat = 228
        let x0 = max(pad, ((bounds.width - (2 * w + gap)) / 2).rounded(.down))      // the grid centred in the tab (user, 2026-10-20)
        rateCard.frame = NSRect(x: x0, y: pad, width: w, height: row1)
        syncCard.frame = NSRect(x: x0 + w + gap, y: pad, width: w, height: row1)
        modelCard?.card.frame = NSRect(x: x0, y: pad + row1 + gap, width: w, height: row2)
        monCard.frame = NSRect(x: modelCard == nil ? x0 : x0 + w + gap, y: pad + row1 + gap, width: w, height: row2)
        knobCard.frame = NSRect(x: x0, y: pad + row1 + gap + row2 + gap, width: 2 * w + gap, height: HardwareTab.knobRow)
        let inner: CGFloat = 18, top = Card.contentTop
        let kw = (2 * w + gap - 2 * inner) / CGFloat(knobRadios.count)
        for (i, (b, _)) in knobRadios.enumerated() { b.frame = NSRect(x: inner + CGFloat(i) * kw, y: top + 6, width: kw - 8, height: 20) }

        // rate: stepper | display, status line under it
        stepper.frame = NSRect(x: inner, y: top + 6, width: 26, height: 76)
        rateDisplay.frame = NSRect(x: inner + 36, y: top + 6, width: w - 2 * inner - 36, height: 76)
        rateStatus.frame = NSRect(x: inner, y: top + 90, width: w - 2 * inner, height: 18)
        followCheck.frame = NSRect(x: inner, y: top + 116, width: w - 2 * inner, height: 20)

        for (i, (b, _)) in syncRadios.enumerated() { b.frame = NSRect(x: inner, y: top + 6 + CGFloat(i) * 32, width: w - 2 * inner, height: 20) }

        modelCard?.layout(width: w, height: row2)

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


    func rebuild() { refresh() }
    static let knobRow: CGFloat = 76
    /// Two 380-pt cards per row, two rows, then the full-width level-knob card.
    var preferredSize: NSSize { return NSSize(width: 2 * Theme.pad + 2 * 380 + 16, height: 2 * Theme.pad + 186 + 16 + 228 + 16 + HardwareTab.knobRow) }

    func refresh() {
        let m = ctx.model
        let c = m.clockSource
        let l = ctx.layout
        rateDisplay.text = Theme.rateText(m.sampleRate)
        let locked = m.rateLocked
        rateDisplay.locked = locked
        if let note = ctx.follower.note {
            rateStatus.stringValue = note
            rateStatus.textColor = Theme.text2
        } else {
            rateStatus.stringValue = locked ? "Locked" : "Not locked"
            rateStatus.textColor = locked ? Theme.green : Theme.red
        }
        followCheck.on = PanelPrefs.followExternalRate
        followCheck.isEnabled = ctx.device?.externalSyncRate != nil
        let i = rates.index(of: m.sampleRate)
        stepper.canUp = (i ?? 0) < rates.count - 1
        stepper.canDown = (i ?? 0) > 0
        for (i, (b, s)) in syncRadios.enumerated() {
            b.on = (c.sync == s)
            b.isEnabled = ctx.profile.hardware.sync[i].enabled(l)
        }
        modelCard?.refresh()
        let dm = (m.number(MAKeys.momentarySwitchSource) ?? 0) != 0
        dmRadio.on = dm; hpRadio.on = !dm
        setSource.isEnabled = !dm
        // A/B text follows the sources set in the sheet: MAMomentarySwitchRoutings entry 0 = A, entry 1 = B. Correction (2026-10-16, run on
        // the real device): entry index = the kext's A/B mode (fMode 0 = A, 1 = B; SetMomentarySwitchRoutingSource idx-1 == fMode), and the
        // kext default {1, 2, 3, 4} then reads A = out 1/2, B = out 3/4, the panel's documented default. The earlier 1/2 reading showed "3/4, -".
        let routes = m.ints(MAKeys.momentarySwitchRoutings)
        let a = routes.count > 0 ? routes[0] : 0, b = routes.count > 1 ? routes[1] : 0
        func name(_ mask: Int) -> String { return ctx.profile.phonesSource(mask: mask)?.short ?? "-" }
        hpNote.stringValue = "A = \(name(a)), LED off\nB = \(name(b)), LED on"
        let step = m.number(MAKeys.monitorKnobStep)                               // nil: a driver without the key (0.1.0)
        for (b, v) in knobRadios { b.on = (step == v); b.isEnabled = step != nil }
    }
    func updateMeters() {}

    /// Debug/demo aid (snapshot): opens the A/B sheet and returns its window.
    func presentABSheet() -> NSWindow? { showSheet(); return sheet?.panel }

    // MARK: headphone A/B sheet (P11 layout; A2-P13 §5 write)
    private func showSheet() {
        guard let window = self.window else { return }
        let routes = ctx.model.ints(MAKeys.momentarySwitchRoutings)
        // An entry that is not one of the model's source masks starts with no choice, as the a/b text shows "-" for it.
        let all = ctx.profile.phonesSources
        func source(_ i: Int) -> UInt32? { return routes.count > i ? all.first { Int($0.mask) == routes[i] }?.mask : nil }
        let s = HPABSheet(a: source(0), b: source(1), sources: all, name: ctx.profile.hardware.abSheetName)
        sheet = s
        s.onSet = { [weak self] a, b in self?.ctx.model.setABSources(a: a, b: b); self?.refresh() }
        window.beginSheet(s.panel) { [weak self] _ in self?.sheet = nil }
    }
}

/// "<Headphone> A/B Source Settings": one of the model's sources for A and for B; the source held by the other side is disabled; Set
/// stays disabled until a choice changes (screenshots P12) and both sides have a source. Values are the kext's source masks.
final class HPABSheet {
    let panel = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 440, height: 214), styleMask: [.titled], backing: .buffered, defer: false)
    private var a: UInt32?, b: UInt32?
    private let original: (UInt32?, UInt32?)
    private var aRadios: [ChoiceButton] = [], bRadios: [ChoiceButton] = []
    private let setButton = FlatButton("Set")
    var onSet: ((UInt32, UInt32) -> Void)?
    private let all: [(String, UInt32)]

    init(a: UInt32?, b: UInt32?, sources: [PhonesSource], name: String) {
        self.a = a; self.b = b; original = (a, b)
        all = sources.map { ($0.name, $0.mask) }
        panel.title = "\(name) A/B Source Settings"
        styleWindow(panel)
        let root = FillView(Theme.surface)
        root.frame = NSRect(x: 0, y: 0, width: 440, height: 214)
        let heading = makeLabel("\(name) A/B Source", size: 14, weight: .semibold, color: Theme.text)
        heading.frame = NSRect(x: 24, y: 20, width: 392, height: 20)
        let text = makeLabel("Select the source for the A and B positions of the \(name) A/B switch.", size: 12, color: Theme.text2)
        text.maximumNumberOfLines = 2; text.lineBreakMode = .byWordWrapping
        text.frame = NSRect(x: 24, y: 46, width: 392, height: 34)
        [heading, text].forEach { root.addSubview($0) }
        func row(_ name: String, y: CGFloat, side: Int) -> [ChoiceButton] {
            let l = makeLabel(name, size: 12, color: Theme.text3); l.frame = NSRect(x: 24, y: y + 1, width: 80, height: 18); root.addSubview(l)
            var radios: [ChoiceButton] = []
            for (i, (t, s)) in all.enumerated() {
                let r = ChoiceButton(radio: t)
                let pitch: CGFloat = all.count <= 3 ? 100 : 76      // three sources: the 0.1.0 geometry; four fit the same row
                r.frame = NSRect(x: 112 + CGFloat(i) * pitch, y: y, width: pitch - 8, height: 20)
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
