// The driver edition this panel was packaged with (BROAD_COMPATIBILITY_PLAN_v2 §11a, user 2026-10-21): the Makefile writes OAEdition into
// Info.plist (`make app BAND=classic|modern|legacy`, the kext Makefile's names). A panel without the key is a Classic one (0.2.x). Only the About
// tab reads it: the panel itself is the same for every edition.
import Foundation

enum PanelEdition: String {
    case classic, modern, legacy

    static let current = PanelEdition(rawValue: (Bundle.main.object(forInfoDictionaryKey: "OAEdition") as? String) ?? "") ?? .classic

    /// About banner caption after the product name; Classic keeps the 0.2.x text (user, 2026-10-21).
    var caption: String {
        switch self {
        case .classic: return "macOS High Sierra (10.13.6) Driver"
        case .modern:  return "Modern Edition  \u{00B7}  macOS 11 to 15 (Intel) Driver"
        case .legacy:  return "Legacy Edition  \u{00B7}  Mac OS X 10.6.8 to 10.8.5 Driver"
        }
    }
    /// "Edition" field of the About tab's version table.
    var versionText: String {
        switch self {
        case .classic: return "Classic (10.13.6)"
        case .modern:  return "Modern (11 to 15, Intel)"
        case .legacy:  return "Legacy (10.6.8 to 10.8.5)"
        }
    }
    /// What sets the edition apart, appended to the About credits (none for Classic, whose text stays as in 0.2.x). Modern: kext built with the
    /// macOS 15.2 SDK (kext/Makefile BAND=modern, R125), linked by kmutil into the auxiliary kernel collection after the user's approval (R126).
    var creditsNote: String? {
        switch self {
        case .classic: return nil
        case .modern:  return "Modern edition: the driver is built with the macOS 15 SDK for the Intel kernels of macOS 11 to 15 and loads after it is "
                            + "allowed in System Settings > Privacy & Security, followed by a restart."
        case .legacy:  return "Legacy edition: the driver is built with the Mac OS X 10.6 SDK for the 32- and 64-bit kernels of 10.6.8 to 10.8.5."
        }
    }
}
