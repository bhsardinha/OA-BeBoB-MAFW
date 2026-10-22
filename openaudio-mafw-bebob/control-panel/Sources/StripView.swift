// PN.3 -- one mixer/output channel strip (wireframe PN1): optional aux knobs, a fader pair with LED meters on the outside and the dB scale
// between the faders, optional pan or balance, ctrl/solo and link/mute buttons, then routing buttons. Every strip of a tab uses the same
// fixed row geometry (StripView.Geometry), so knobs, faders and buttons line up across strips; a strip without a row leaves it empty.
// The strip holds no model state: the owning tab sets the state and receives the callbacks and writes DeviceModel (A2-P13 index rules).
// Sides are 0 = left channel, 1 = right channel.
import AppKit

struct StripFeatures: OptionSet {
    let rawValue: Int
    static let aux       = StripFeatures(rawValue: 1)       // aux L / aux R knobs (mixer strips)
    static let pan       = StripFeatures(rawValue: 2)       // pan L / pan R knobs (mixer input strips)
    static let balance   = StripFeatures(rawValue: 4)       // one balance knob (output strips)
    static let ctrl      = StripFeatures(rawValue: 8)       // rotary-encoder button
    static let solo      = StripFeatures(rawValue: 16)
    static let route     = StripFeatures(rawValue: 32)      // out 1/2, out 3/4
    static let mainAux   = StripFeatures(rawValue: 64)      // main / aux toggle (output strips)
    static let mon       = StripFeatures(rawValue: 128)     // phones: monitor source 1/2, 3/4, aux
    static let faders    = StripFeatures(rawValue: 256)     // fader handles (else scale and meters only)
    static let linkMute  = StripFeatures(rawValue: 512)
}

final class StripView: FlippedView {
    enum Kind { case mixer, output }

    /// Row positions (top-down) shared by every strip of one tab.
    struct Geometry {
        let titleY: CGFloat = 12
        let knobY: CGFloat = 36             // caption; knobs at +12 (32 high); values at +46
        let faderY: CGFloat = 100
        let faderHeight: CGFloat = 176
        var readoutY: CGFloat { return faderY + faderHeight + 4 }
        let lowKnobY: CGFloat?              // pan row (mixer)
        /// Where a pan pair sits: the mixer's pan row, or on an output strip the bottom area (the aux output's pan, MAAuxOutputPan2).
        var panY: CGFloat { return lowKnobY ?? bottomCaptionY }
        let button1Y: CGFloat
        let button2Y: CGFloat
        let bottomCaptionY: CGFloat
        let bottomY: CGFloat
        let height: CGFloat

        /// `bottomRows` > 1: extra 24-pt rows under the bottom buttons (a model with four headphone sources shows them 2 x 2); every strip of
        /// the tab gets the same value so the rows line up.
        init(_ kind: Kind, bottomRows: Int = 1) {
            let afterReadout: CGFloat = 100 + 176 + 4 + 12
            switch kind {
            case .mixer:
                lowKnobY = afterReadout + 10
                button1Y = afterReadout + 10 + 58 + 10
            case .output:
                lowKnobY = nil
                button1Y = afterReadout + 14
            }
            button2Y = button1Y + 26
            bottomCaptionY = button2Y + 20 + 14
            bottomY = bottomCaptionY + 13
            height = bottomY + 20 + 14 + CGFloat(max(0, bottomRows - 1)) * 24
        }
        // x positions in a strip of Theme.stripWidth: meter | fader | scale | fader | meter, symmetric around the centre.
        static let faderCenters: [CGFloat] = [25, 59]
        static let meterX: [CGFloat] = [8, 72]
        static let meterWidth: CGFloat = 4
        static let scaleX: CGFloat = 36, scaleWidth: CGFloat = 12
    }

    let features: StripFeatures
    let kind: Kind
    let geometry: Geometry
    var title: String { didSet { needsDisplay = true } }
    let faders = [Fader(), Fader()]
    let meters = [MeterBar(), MeterBar()]
    let scale = FaderScale()
    /// Fader law of the strip's level key (DeviceModel.law): phones follow the dB-linear monitor law (plan v7 AP-5.4, phones only since R127);
    /// every other strip, mixer and output tab alike, the console law.
    let law: FaderLaw
    var auxKnobs: [Knob] = []
    var panKnobs: [Knob] = []
    let balanceKnob = Knob()
    let ctrlButton = PillButton("ctrl"), soloButton = PillButton("solo")
    let linkButton = PillButton("link"), muteButton = PillButton("mute")
    var routeButtons: [PillButton] = []        // the profile's routing destinations (FW1814 out 1/2, 3/4; Audiophile 1/2, 3/4, spd)
    private let routeLabels: [String]
    private let monSources: [PhonesSource]
    let mainAuxButton = PillButton("main")
    var monButtons: [PillButton] = []          // the profile's headphone sources
    private var stripEnabled = true

    var onAux: ((Int, Int) -> Void)?
    var onLevel: ((Int, Int) -> Void)?
    var onPan: ((Int, Int) -> Void)?
    var onBalance: ((Int) -> Void)?
    var onLink: ((Bool) -> Void)?
    var onMute: ((Bool) -> Void)?
    var onSolo: ((Bool) -> Void)?
    var onCtrl: ((Bool) -> Void)?
    var onRoute: ((Int, Bool) -> Void)?
    var onMainAux: ((Bool) -> Void)?
    var onMon: ((UInt32) -> Void)?             // source mask
    var onCommit: (() -> Void)?

    init(title: String, features: StripFeatures, kind: Kind, law: FaderLaw = .console, routeLabels: [String] = [], monSources: [PhonesSource] = [],
         bottomRows: Int = 1) {
        self.features = features
        self.law = law
        self.routeLabels = routeLabels
        self.monSources = monSources
        self.kind = kind
        self.geometry = Geometry(kind, bottomRows: bottomRows)
        self.title = title
        super.init(frame: NSRect(x: 0, y: 0, width: Theme.stripWidth, height: geometry.height))
        build()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func place(_ v: NSView, _ r: NSRect) { v.frame = r; addSubview(v) }

    private func build() {
        let g = geometry
        let cx = Geometry.faderCenters
        if features.contains(.aux) {
            for side in 0..<2 {
                let k = Knob(); k.minValue = MARange.levelMin; k.maxValue = MARange.levelMax; k.defaultValue = MARange.levelMin
                k.onChange = { [weak self] v in self?.onAux?(side, v); self?.needsDisplay = true }
                k.onCommit = { [weak self] in self?.onCommit?() }
                auxKnobs.append(k)
                place(k, NSRect(x: cx[side] - 16, y: g.knobY + 12, width: 32, height: 32))
            }
        }
        if features.contains(.balance) {
            balanceKnob.minValue = -64; balanceKnob.maxValue = 64; balanceKnob.defaultValue = 0; balanceKnob.bipolar = true
            balanceKnob.onChange = { [weak self] v in self?.onBalance?(v); self?.needsDisplay = true }
            balanceKnob.onCommit = { [weak self] in self?.onCommit?() }
            place(balanceKnob, NSRect(x: Theme.stripWidth / 2 - 16, y: g.knobY + 12, width: 32, height: 32))
        }
        for side in 0..<2 {
            let m = meters[side]
            place(m, NSRect(x: Geometry.meterX[side], y: g.faderY + Fader.trackPad, width: Geometry.meterWidth, height: g.faderHeight - 2 * Fader.trackPad))
            if features.contains(.faders) {
                let f = faders[side]
                f.law = law
                f.onChange = { [weak self] v in self?.onLevel?(side, v); self?.needsDisplay = true }
                f.onCommit = { [weak self] in self?.onCommit?() }
                place(f, NSRect(x: cx[side] - 11, y: g.faderY, width: 22, height: g.faderHeight))
            }
        }
        scale.law = law
        place(scale, NSRect(x: Geometry.scaleX, y: g.faderY, width: Geometry.scaleWidth, height: g.faderHeight))
        if features.contains(.pan) {
            let y = g.panY
            for side in 0..<2 {
                let k = Knob(); k.minValue = MARange.panMin; k.maxValue = MARange.panMax; k.bipolar = true
                k.defaultValue = side == 0 ? MARange.panMin : MARange.panMax
                k.onChange = { [weak self] v in self?.onPan?(side, v); self?.needsDisplay = true }
                k.onCommit = { [weak self] in self?.onCommit?() }
                panKnobs.append(k)
                place(k, NSRect(x: cx[side] - 16, y: y + 12, width: 32, height: 32))
            }
        }
        // Buttons: two 34-pt slots per row (left x 6, right x 44).
        if features.contains(.ctrl) { place(ctrlButton, NSRect(x: 6, y: g.button1Y, width: 34, height: 20)) }
        if features.contains(.solo) { place(soloButton, NSRect(x: 44, y: g.button1Y, width: 34, height: 20)) }
        if features.contains(.linkMute) {
            place(linkButton, NSRect(x: 6, y: g.button2Y, width: 34, height: 20))
            place(muteButton, NSRect(x: 44, y: g.button2Y, width: 34, height: 20))
        }
        soloButton.onColor = Theme.solo; soloButton.solid = true
        muteButton.onColor = Theme.mute; muteButton.solid = true
        ctrlButton.onClick = { [weak self] b in self?.onCtrl?(b.isOn) }
        soloButton.onClick = { [weak self] b in self?.onSolo?(b.isOn) }
        linkButton.onClick = { [weak self] b in self?.onLink?(b.isOn) }
        muteButton.onClick = { [weak self] b in self?.onMute?(b.isOn) }
        // n buttons share the 72 pt between the 6-pt margins: 2 -> 34 pt at a 38-pt pitch (the 0.1.0 geometry), 3 -> 21.3 pt.
        if features.contains(.route) {
            let n = CGFloat(max(1, routeLabels.count)), gap: CGFloat = 4
            let w = ((Theme.stripWidth - 12 - (n - 1) * gap) / n).rounded(.down)
            for (i, t) in routeLabels.enumerated() {
                let b = PillButton(t); b.onClick = { [weak self] btn in self?.onRoute?(i, btn.isOn) }
                routeButtons.append(b)
                place(b, NSRect(x: 6 + CGFloat(i) * (w + gap), y: g.bottomY, width: w, height: 20))
            }
        }
        if features.contains(.mainAux) {
            StripView.styleMainAux(mainAuxButton, aux: false)
            mainAuxButton.onClick = { [weak self] b in
                StripView.styleMainAux(b, aux: b.isOn)
                self?.onMainAux?(b.isOn)
            }
            place(mainAuxButton, NSRect(x: 6, y: g.bottomY, width: 72, height: 20))
        }
        // Up to 3 sources: one row, 22 pt at a 25-pt pitch (the 0.1.0 geometry). More: a 2-column grid of 34-pt buttons (link/mute size).
        if features.contains(.mon) {
            let grid = monSources.count > 3
            let n = CGFloat(max(1, monSources.count)), gap: CGFloat = 3
            let w = ((Theme.stripWidth - 12 - (n - 1) * gap) / n).rounded(.down)
            for (i, src) in monSources.enumerated() {
                let b = PillButton(src.button); b.momentary = true
                b.onClick = { [weak self] _ in self?.onMon?(src.mask) }
                monButtons.append(b)
                if grid { place(b, NSRect(x: 6 + CGFloat(i % 2) * 38, y: g.bottomY + CGFloat(i / 2) * 24, width: 34, height: 20)) }
                else { place(b, NSRect(x: 6 + CGFloat(i) * (w + gap), y: g.bottomY, width: w, height: 20)) }
            }
        }
    }

    /// Main/aux selector of an output pair: both states are lit, main in the accent colour, aux in green.
    static func styleMainAux(_ b: PillButton, aux: Bool) {
        b.title = aux ? "aux" : "main"
        b.onColor = Theme.green
        b.offColor = Theme.accent
    }

    // MARK: drawing (surface, title, captions, value readouts)

    override func draw(_ dirtyRect: NSRect) {
        let g = geometry
        let w = bounds.width
        Theme.fill(bounds, radius: Theme.radius, Theme.surface)
        let on = stripEnabled
        Theme.draw(title, in: NSRect(x: 2, y: g.titleY, width: w - 4, height: 16), font: Theme.font(11, .semibold), color: on ? Theme.text : Theme.text4)
        let capColor = on ? Theme.text3 : Theme.text4
        let valColor = on ? Theme.text2 : Theme.text4
        let cx = Geometry.faderCenters
        if features.contains(.aux) {
            Theme.caption("aux", in: NSRect(x: 0, y: g.knobY, width: w, height: 10), color: capColor)
            for side in 0..<2 {
                Theme.draw(Theme.levelText(auxKnobs[side].value), in: NSRect(x: cx[side] - 18, y: g.knobY + 46, width: 36, height: 11), font: Theme.mono(9), color: valColor)
            }
        }
        if features.contains(.balance) {
            Theme.caption("bal", in: NSRect(x: 0, y: g.knobY, width: w, height: 10), color: capColor)
            Theme.draw(Theme.balanceText(balanceKnob.value), in: NSRect(x: 0, y: g.knobY + 46, width: w, height: 11), font: Theme.mono(9), color: valColor)
        }
        if features.contains(.faders) {
            for side in 0..<2 {
                Theme.draw(Theme.levelText(faders[side].value, law: faders[side].law), in: NSRect(x: cx[side] - 18, y: g.readoutY, width: 36, height: 11), font: Theme.mono(9), color: valColor)
            }
        }
        if features.contains(.pan) {
            let y = g.panY
            Theme.caption("pan", in: NSRect(x: 0, y: y, width: w, height: 10), color: capColor)
            for side in 0..<2 {
                Theme.draw(Theme.panText(panKnobs[side].value), in: NSRect(x: cx[side] - 18, y: y + 46, width: 36, height: 11), font: Theme.mono(9), color: valColor)
            }
        }
        if features.contains(.route) { Theme.caption("out", in: NSRect(x: 0, y: g.bottomCaptionY, width: w, height: 10), color: capColor) }
        if features.contains(.mon) { Theme.caption("monitor", in: NSRect(x: 0, y: g.bottomCaptionY, width: w, height: 10), color: capColor) }
    }

    // MARK: state setters (the tab calls these from DeviceModel)

    func setLevels(_ l: Int, _ r: Int) {
        if faders[0].value != l || faders[1].value != r { faders[0].value = l; faders[1].value = r; needsDisplay = true }
    }
    func setAux(_ l: Int, _ r: Int) {
        if auxKnobs.count == 2 && (auxKnobs[0].value != l || auxKnobs[1].value != r) { auxKnobs[0].value = l; auxKnobs[1].value = r; needsDisplay = true }
    }
    func setPan(_ l: Int, _ r: Int) {
        if panKnobs.count == 2 && (panKnobs[0].value != l || panKnobs[1].value != r) { panKnobs[0].value = l; panKnobs[1].value = r; needsDisplay = true }
    }
    func setBalance(_ b: Int) {
        if balanceKnob.value != b { balanceKnob.value = b; needsDisplay = true }
    }
    func setFlags(link: Bool, mute: Bool, solo: Bool) { linkButton.isOn = link; muteButton.isOn = mute; soloButton.isOn = solo }
    func setCtrl(enabled: Bool, on: Bool) { ctrlButton.isEnabled = enabled && stripEnabled; ctrlButton.isOn = enabled && on }
    func setRoute(mask: Int) { for (i, b) in routeButtons.enumerated() { b.isOn = (mask >> i) & 1 != 0 } }
    func setMainAux(aux: Bool) { mainAuxButton.isOn = aux; StripView.styleMainAux(mainAuxButton, aux: aux) }
    func setMon(_ mask: UInt32) {
        for (i, b) in monButtons.enumerated() { b.isOn = (mask == monSources[i].mask) }
    }
    func setMeters(_ l: Int, _ r: Int) { meters[0].db = l; meters[1].db = r }

    /// Greys the whole strip (ADAT 5-8 at 88.2/96 kHz).
    func setStripEnabled(_ on: Bool) {
        if stripEnabled != on { stripEnabled = on; needsDisplay = true }
        for f in faders { f.isEnabled = on }
        (auxKnobs + panKnobs + [balanceKnob]).forEach { $0.isEnabled = on }
        for b in [soloButton, linkButton, muteButton, mainAuxButton] + routeButtons + monButtons { b.isEnabled = on }
        for m in meters { m.isEnabled = on }
        scale.isEnabled = on
        if !on { ctrlButton.isEnabled = false }
    }
}
