// SKELETON -- plan task M-K (PROVENANCE-MATRIX.xlsx / Build tab). Status: NOT STARTED.
// Role: Static sNubDictionary keyed by GUIDString; ReplaceExistingMetaNub; multi-device
// Spec: docs/system-map/A1-ghidra/Q7-fwmetanub-lifecycle.md
// Spec: docs/system-map/C1-lifecycle-state-machine.md
// Reference: FWMetaNub::ReplaceExistingMetaNub @0x1cd5a; sNubDictionary; FWMetaNub::stop @0x1ba84
// Acceptance (sheet): Duas unidades, dois engines independentes
// Rules: mirror _extracted/, cite every value (binary offset / FFADO file:line / dated capture), label guesses; NULL-safe free(); no partial hardware tests.

#include "FWNubRegistry.h"
