// CFW1814MixerChannelPairView::MapMessageToHWPairID
// Entry: 000556a8
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814MixerChannelPairView::MapMessageToHWPairID(long) */

char __thiscall
CFW1814MixerChannelPairView::MapMessageToHWPairID(CFW1814MixerChannelPairView *this,long param_1)

{
  return (param_1 == 0x84) + '\x01';
}
