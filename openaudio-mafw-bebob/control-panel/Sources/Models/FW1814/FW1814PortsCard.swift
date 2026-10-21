// FW1814 digital-port card of the hardware tab (module Models/FW1814): optical in/out ADAT or S/PDIF, S/PDIF input port, ADAT S/MUX. Every
// widget is a read-modify-write of one MAClockSource bit (P-ClockUI-encoding §2; MAClockSource.wireValue keeps the gate); greying follows the
// FW1814 layout rules (device truth, P12). Moved unchanged from the 0.1.0 HardwareTab (AP-5.1).
import AppKit

final class FW1814PortsCard: HardwareCard {
    let card = Card(title: "Digital Ports")
    private let ctx: PanelContext
    private let inAdat = ChoiceButton(radio: "ADAT"), inSpdif = ChoiceButton(radio: "S/PDIF")
    private let outAdat = ChoiceButton(radio: "ADAT"), outSpdif = ChoiceButton(radio: "S/PDIF")
    private let portOptical = ChoiceButton(radio: "Optical"), portCoax = ChoiceButton(radio: "Coaxial")
    private let smux = ChoiceButton(check: "ADAT S/MUX")
    private var portLabels: [NSTextField] = []
    var onChange: (() -> Void)?

    init(context: PanelContext) {
        ctx = context
        let m = ctx.model
        inAdat.onAction = { [weak self] in m.updateClock { $0.adatIn = true }; self?.afterClockWrite() }
        inSpdif.onAction = { [weak self] in m.updateClock { $0.adatIn = false }; self?.afterClockWrite() }
        outAdat.onAction = { [weak self] in m.updateClock { $0.adatOut = true }; self?.afterClockWrite() }
        outSpdif.onAction = { [weak self] in m.updateClock { $0.adatOut = false }; self?.afterClockWrite() }
        portOptical.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = false }; self?.changed() }
        portCoax.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = true }; self?.changed() }
        smux.onAction = { [weak self, weak smux] in
            let on = smux?.on ?? false
            m.updateClock { $0.smux = on }
            self?.changed()
        }
        portLabels = ["Input", "Output", "S/PDIF input"].map { makeLabel($0, size: 12, color: Theme.text3) }
        (portLabels as [NSView] + [inAdat, inSpdif, outAdat, outSpdif, portOptical, portCoax, smux]).forEach { card.addSubview($0) }
    }

    private func changed() { if let c = onChange { c() } else { refresh() } }

    /// Optical in/out changed: S/MUX follows the device rule (forced on at 88.2/96 kHz with ADAT), then redraw everything.
    private func afterClockWrite() {
        let l = ctx.layout
        if l.smuxWanted != ctx.model.clockSource.smux { ctx.model.updateClock { $0.smux = l.smuxWanted } }
        changed()
    }

    func layout(width w: CGFloat, height: CGFloat) {
        let inner: CGFloat = 18, top = Card.contentTop
        let labelW: CGFloat = 112, col1 = inner + labelW, col2 = col1 + 96
        let rowsY: [CGFloat] = [top + 6, top + 42, top + 78]
        for (i, l) in portLabels.enumerated() { l.frame = NSRect(x: inner, y: rowsY[i] + 1, width: labelW - 8, height: 18) }
        for (i, pair) in [(inAdat, inSpdif), (outAdat, outSpdif), (portOptical, portCoax)].enumerated() {
            pair.0.frame = NSRect(x: col1, y: rowsY[i], width: 92, height: 20)
            pair.1.frame = NSRect(x: col2, y: rowsY[i], width: 92, height: 20)
        }
        smux.frame = NSRect(x: col1, y: top + 126, width: 160, height: 20)
    }

    func refresh() {
        let c = ctx.model.clockSource
        let l = ctx.layout
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
    }
}
