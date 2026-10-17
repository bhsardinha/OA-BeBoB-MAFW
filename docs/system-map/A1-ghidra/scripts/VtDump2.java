import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
public class VtDump2 extends GhidraScript {
  public void run() throws Exception {
    String[] a=getScriptArgs(); String cls=a[0];
    int[] slots=new int[a.length-1];
    for (Symbol s : currentProgram.getSymbolTable().getAllSymbols(false)) {
      String n=s.getName(true);
      if (n.equals(cls+"::vtable")) {
        for (int i=1;i<a.length;i++){ int sl=Integer.decode(a[i]);
          long v=currentProgram.getMemory().getLong(s.getAddress().add(0x10+sl));
          Function f=getFunctionAt(toAddr(v));
          println(String.format("%s slot 0x%x -> %x %s",cls,sl,v,f==null?"?":f.getName(true)));}
      }
    }
  }
}
