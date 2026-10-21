// PN.5 -- the about tab (wireframe PN1): the front-panel banner and one card of the same width with the version table, a credits paragraph and
// the source link as a centred button (the reference's three M-Audio web links are gone, user 2026-10-17); device fields decoded by FirmwareInfo.swift. The banner image is the product photo with its white background removed (Resources/fw1814-front.png).
import AppKit
import IOKit
import IOKit.kext

/// About banner: the model's front-panel photo (profile.aboutPhoto, background removed, e.g. Resources/fw1814-front.png) on a dark panel;
/// without a photo the model's name.
private final class Banner: FlippedView {
    private let profile: DeviceProfile
    private let photo: NSImage?
    init(profile: DeviceProfile) { self.profile = profile; photo = NSImage(named: NSImage.Name(profile.aboutPhoto)); super.init(frame: .zero) }
    required init?(coder: NSCoder) { fatalError("unused") }
    override func draw(_ dirtyRect: NSRect) {
        let path = NSBezierPath(roundedRect: bounds, xRadius: 14, yRadius: 14)
        NSGradient(starting: Theme.hex(0x22252A), ending: Theme.hex(0x0E0F11))?.draw(in: path, angle: 90)
        let caption = NSRect(x: 0, y: bounds.maxY - 40, width: bounds.width, height: 18)
        if let img = photo {
            // native size (sharp on 1x screens), centred above the caption, with a soft shadow under the unit
            let sz = img.size
            let r = NSRect(x: ((bounds.width - sz.width) / 2).rounded(), y: ((caption.minY - sz.height) / 2 + 6).rounded(), width: sz.width, height: sz.height)
            NSGraphicsContext.saveGraphicsState()
            let sh = NSShadow(); sh.shadowColor = NSColor(white: 0, alpha: 0.75); sh.shadowBlurRadius = 18; sh.shadowOffset = NSSize(width: 0, height: -8); sh.set()
            img.draw(in: r, from: .zero, operation: .sourceOver, fraction: 1, respectFlipped: true, hints: nil)
            NSGraphicsContext.restoreGraphicsState()
        } else {
            Theme.draw(profile.aboutTitle, in: NSRect(x: 0, y: 40, width: bounds.width, height: 40), font: Theme.font(30, .semibold), color: Theme.text)
        }
        Theme.draw(profile.aboutCaption, in: caption, font: Theme.font(12), color: Theme.text3)
    }
}

final class AboutTab: FlippedView, PanelTab {
    private let ctx: PanelContext
    private let banner: Banner
    private let info = Card(title: "Versions")
    private var fields: [String: NSTextField] = [:]
    private var rows: [(key: NSTextField, value: NSTextField)] = []      // two per line, in the order of `names`
    private let rule = Rule()
    private let creditsLabel = makeLabel("", size: 12, color: Theme.text2)
    private let projectButton = FlatButton(AboutTab.projectURL.replacingOccurrences(of: "https://", with: ""))
    /// Version fields, two per line: Package + Panel, Driver + Hardware, Bootloader + Firmware, GUID. The package (the installer's release number,
    /// VERSION in the tree) and the components have separate numbers (user, 2026-10-21); the Makefile writes the package one as OAPackageVersion.
    private static let names = ["package", "panel", "driver", "hardware", "bootloader", "firmware", "GUID"]

    /// Future home of the source (user's GitHub account bhsardinha, 2026-10-17). **Not published yet**; the repository name is assumed to match this one.
    static let projectURL = "https://github.com/bhsardinha/openaudio-fw1814"
    /// Credits paragraph under the version table. Versions are those of the reference bundles in `_extracted/` (the 1.11.2 installer carries kext
    /// `com.m-audio.driver.firewire` 1.11.3, panel 1.5.2, MIDI plug-in 1.11.2) and of the Snow Leopard rig (driver 1.9, panel 1.4) used for the live
    /// DTrace/ioreg captures; the tools are the ones of NEW_PLAN Parts A/B.
    static let creditsText = "Based on the M-Audio FireWire driver 1.11.2 (kext 1.11.3, panel 1.5.2, MIDI plug-in 1.11.2, 2014), with its behaviour "
        + "checked against driver 1.9 (panel 1.4). Created by Bernardo Sardinha with Claude (Anthropic). Reverse engineered with Ghidra 12.1.4, otool "
        + "and DTrace (live captures from a Snow Leopard rig), and cross-checked with FFADO, Linux snd-bebob and macfw."
    private static let versionRowH: CGFloat = 25, bannerH: CGFloat = 176, inset: CGFloat = 18
    private static let ruleGap: CGFloat = 14, textGap: CGFloat = 14, buttonGap: CGFloat = 18, buttonH: CGFloat = 28, bottom: CGFloat = 20
    private static var versionsHeight: CGFloat { return Card.contentTop + CGFloat((names.count + 1) / 2) * versionRowH }

    private static let creditsAttributed: NSAttributedString = {
        let ps = NSMutableParagraphStyle(); ps.lineSpacing = 4; ps.alignment = .natural
        return NSAttributedString(string: creditsText, attributes: [.font: Theme.font(12), .foregroundColor: Theme.text2, .paragraphStyle: ps])
    }()
    /// Height of the credits paragraph laid out across a card of width `cardWidth` (NSTextField keeps 2 pt of line-fragment padding each side).
    private static func creditsTextHeight(cardWidth: CGFloat) -> CGFloat {
        let w = cardWidth - 2 * inset - 4
        return ceil(creditsAttributed.boundingRect(with: NSSize(width: w, height: 1000), options: [.usesLineFragmentOrigin, .usesFontLeading]).height) + 4
    }
    private static func cardHeight(width: CGFloat) -> CGFloat {
        return versionsHeight + ruleGap + 1 + textGap + creditsTextHeight(cardWidth: width) + buttonGap + buttonH + bottom
    }

    /// One-pixel separator between the version table and the credits.
    private final class Rule: NSView {
        override func draw(_ dirtyRect: NSRect) { Theme.hairline.setFill(); bounds.fill() }
    }

    init(context: PanelContext) {
        ctx = context
        banner = Banner(profile: context.profile)
        super.init(frame: .zero)
        build()
        refresh()
    }
    required init?(coder: NSCoder) { fatalError("unused") }

    private func build() {
        [banner, info].forEach { addSubview($0) }
        for n in AboutTab.names {
            let k = makeLabel(n.prefix(1).uppercased() + n.dropFirst(), size: 12, color: Theme.text3)
            let v = makeLabel("-", size: 12, color: Theme.text); v.font = Theme.mono(12); v.isSelectable = true
            fields[n] = v
            rows.append((k, v)); info.addSubview(k); info.addSubview(v)
        }
        creditsLabel.attributedStringValue = AboutTab.creditsAttributed
        creditsLabel.lineBreakMode = .byWordWrapping; creditsLabel.maximumNumberOfLines = 0; creditsLabel.isSelectable = true
        creditsLabel.cell?.wraps = true
        projectButton.onAction = { if let u = URL(string: AboutTab.projectURL) { NSWorkspace.shared.open(u) } }
        [rule, creditsLabel, projectButton].forEach { info.addSubview($0) }
        layoutParts()
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        layoutParts()
    }

    private func layoutParts() {
        let pad = Theme.pad, gap: CGFloat = 12, inset = AboutTab.inset
        let total = max(704, min(856, bounds.width - 2 * pad))
        let x0 = max(pad, ((bounds.width - total) / 2).rounded(.down))                  // centred in the tab (user, 2026-10-20)
        banner.frame = NSRect(x: x0, y: pad, width: total, height: AboutTab.bannerH)
        info.frame = NSRect(x: x0, y: pad + AboutTab.bannerH + gap, width: total, height: AboutTab.cardHeight(width: total))   // same width as the banner
        // version table: two columns, key 80 pt then the value
        let colW = (total - 2 * inset) / 2
        for (i, r) in rows.enumerated() {
            let x = inset + CGFloat(i % 2) * colW, y = Card.contentTop + 2 + CGFloat(i / 2) * AboutTab.versionRowH
            r.key.frame = NSRect(x: x, y: y, width: 78, height: 18)
            r.value.frame = NSRect(x: x + 80, y: y, width: colW - 88, height: 18)
        }
        // credits: one paragraph across the whole card, then the source link as a centred button
        var y = AboutTab.versionsHeight + AboutTab.ruleGap
        rule.frame = NSRect(x: inset, y: y, width: total - 2 * inset, height: 1)
        y += 1 + AboutTab.textGap
        let th = AboutTab.creditsTextHeight(cardWidth: total)
        creditsLabel.preferredMaxLayoutWidth = total - 2 * inset
        creditsLabel.frame = NSRect(x: inset, y: y, width: total - 2 * inset, height: th)
        y += th + AboutTab.buttonGap
        let bw = projectButton.intrinsicContentSize.width + 36
        projectButton.frame = NSRect(x: ((total - bw) / 2).rounded(), y: y, width: bw, height: AboutTab.buttonH)
    }

    func rebuild() { refresh() }
    func updateMeters() {}
    /// A 760-pt column: banner, then one card of the same width with the version table and the credits.
    var preferredSize: NSSize { return NSSize(width: 760 + 2 * Theme.pad, height: Theme.pad + AboutTab.bannerH + 12 + AboutTab.cardHeight(width: 760) + Theme.pad) }

    func refresh() {
        let bundle = Bundle.main
        let ver = (bundle.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String) ?? "-"
        let build = (bundle.object(forInfoDictionaryKey: "CFBundleVersion") as? String) ?? "-"
        fields["panel"]?.stringValue = "\(ver) (build \(build))"
        fields["package"]?.stringValue = (bundle.object(forInfoDictionaryKey: "OAPackageVersion") as? String) ?? "-"
        fields["driver"]?.stringValue = AboutTab.driverVersion() ?? "-"
        let data = ctx.model.cache["MAFirmwareInfo"] as? Data
        if let d = data, let info = FirmwareInfo.parse(d) {
            fields["bootloader"]?.stringValue = FirmwareInfo.display(date: info.bootloaderDate, time: info.bootloaderTime)
            fields["firmware"]?.stringValue = FirmwareInfo.display(date: info.firmwareDate, time: info.firmwareTime)
            // Firmware older than the build the official bundle ships for this model: the driver does not update firmware (plan v6), so say so.
            let min = ctx.profile.minimumFirmware
            if (info.firmwareDate + info.firmwareTime) < min, min.count == 14 {
                let m = Array(min)
                fields["firmware"]?.textColor = Theme.red
                fields["firmware"]?.stringValue += "  (below \(String(m[0..<4]))-\(String(m[4..<6]))-\(String(m[6..<8])))"
                fields["firmware"]?.toolTip = "This unit's firmware is older than the official 1.11.2 build for this model (\(String(m[0..<4]))-\(String(m[4..<6]))-\(String(m[6..<8])) \(String(m[8..<10])):\(String(m[10..<12]))). This driver does not update firmware; connecting the unit once to a Mac with the official 1.11.2 driver does."
            }
            fields["hardware"]?.stringValue = "model \(info.hardwareModel) version \(info.hardwareRevision)"
            fields["GUID"]?.stringValue = ctx.device?.info.guid.isEmpty == false ? ctx.device!.info.guid : info.guid
        }
    }

    /// Version of the loaded clone kext (KextManager is not used: IOKit's registry has no version for us, so read the kext's Info.plist
    /// through the system extension list).
    static func driverVersion() -> String? {
        let ids = ["com.openaudio.OpenAudioBeBoBMAFW"]
        guard let info = KextManagerCopyLoadedKextInfo(ids as CFArray, ["CFBundleVersion"] as CFArray)?.takeRetainedValue() as? [String: Any],
              let k = info[ids[0]] as? [String: Any] else { return nil }
        return k["CFBundleVersion"] as? String
    }
}
