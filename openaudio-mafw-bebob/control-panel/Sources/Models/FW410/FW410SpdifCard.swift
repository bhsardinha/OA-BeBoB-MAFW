// FW410 S/PDIF card of the hardware tab (module Models/FW410): the active S/PDIF input, coaxial or optical (official panel "active spdif
// input"; manual p.7, p.21: the output always goes to both connectors). One MAClockSource bit: bit 2 = coaxial (capture 103: coax 0x40000004,
// optical 0x40000000; kext SetClockSourceInternal asm 0x14eda: sync type 4 / 3).
import AppKit

final class FW410SpdifCard: HardwareCard {
    let card = Card(title: "S/PDIF Input")
    private let ctx: PanelContext
    private let coax = ChoiceButton(radio: "Coaxial (RCA)"), optical = ChoiceButton(radio: "Optical (TOSLink)")
    private let note = makeLabel("The S/PDIF output always plays on both connectors.\nThe front S/PDIF In LED flashes when a signal\nis on the other input.",
                                 size: 11, color: Theme.text3)
    var onChange: (() -> Void)?

    init(context: PanelContext) {
        ctx = context
        let m = ctx.model
        coax.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = true }; self?.changed() }
        optical.onAction = { [weak self] in m.updateClock { $0.spdifCoaxial = false }; self?.changed() }
        note.maximumNumberOfLines = 3
        [coax, optical, note].forEach { card.addSubview($0) }
    }

    private func changed() { if let c = onChange { c() } else { refresh() } }

    func layout(width w: CGFloat, height: CGFloat) {
        let inner: CGFloat = 18, top = Card.contentTop
        coax.frame = NSRect(x: inner, y: top + 6, width: w - 2 * inner, height: 20)
        optical.frame = NSRect(x: inner, y: top + 38, width: w - 2 * inner, height: 20)
        note.frame = NSRect(x: inner, y: top + 76, width: w - 2 * inner, height: 48)
    }

    func refresh() {
        let c = ctx.model.clockSource
        coax.on = c.spdifCoaxial; optical.on = !c.spdifCoaxial
    }
}
