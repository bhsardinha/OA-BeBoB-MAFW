import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
// Dump every vtable slot 0x118..<max> of each class given as script arg (slot = entry at vtable_symbol + 0x10 + slot).
public class VtDumpAll extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    int max = Integer.decode(a[0]);
    for (int i = 1; i < a.length; i++) {
      String cls = a[i];
      for (Symbol s : currentProgram.getSymbolTable().getAllSymbols(false)) {
        if (!s.getName(true).equals(cls + "::vtable")) continue;
        for (int sl = 0x118; sl <= max; sl += 8) {
          long v = currentProgram.getMemory().getLong(s.getAddress().add(0x10 + sl));
          if (v == 0) break;
          Function f = getFunctionAt(toAddr(v));
          println(String.format("%s slot 0x%x -> %x %s", cls, sl, v, f == null ? "?" : f.getName(true)));
        }
      }
    }
  }
}
