#!/usr/bin/python3 -W ignore
"""Update task status/notes on the `Build` tab of PROVENANCE-MATRIX-AUDIOPHILE.xlsx (the Audiophile-phase tracker).
Copy of openaudio-fw1814/scripts/plan-status.py (that tree is not edited by the multi-model work), with the
workbook path and the extra status "Aguarda o usuário".

Usage:
  scripts/ap-status.py list
  scripts/ap-status.py set <ID> "<Status>" "<dated note>" [<ID> "<Status>" "<note>" ...]

Allowed Status: Não iniciado | Autorizado (pré-trabalho) | Em andamento | Concluído | Aguarda aprovação do plano |
                Cancelado (corte de escopo) | Aguarda o usuário | Sob demanda (uso real)
"Sob demanda (uso real)" (R123): a test that runs only if real use shows a problem; excluded from the totals like "Cancelado".
Keeps a backup in $TMPDIR/ap-status-backup.xlsx, appends the note to column I (Notas), never touches formulas, and
re-checks the workbook through LibreOffice for #REF!/#VALUE!/#NAME? (skill close-item, step 3).
"""
import datetime, os, shutil, subprocess, sys, tempfile
import openpyxl

XLSX = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "PROVENANCE-MATRIX-AUDIOPHILE.xlsx")
ALLOWED = ["Não iniciado", "Autorizado (pré-trabalho)", "Em andamento", "Concluído", "Aguarda aprovação do plano",
           "Cancelado (corte de escopo)", "Aguarda o usuário", "Sob demanda (uso real)"]


def main():
    wb = openpyxl.load_workbook(XLSX)
    ws = wb["Build"]
    rows = {ws.cell(r, 1).value: r for r in range(2, ws.max_row + 1) if ws.cell(r, 1).value}
    if len(sys.argv) < 2 or sys.argv[1] == "list":
        for k, r in rows.items():
            print(f"{k:5} {ws.cell(r, 6).value:30} {str(ws.cell(r, 3).value)[:70]}")
        return
    args = sys.argv[2:]
    if len(args) % 3:
        sys.exit("set needs <ID> <Status> <note> triples")
    shutil.copy(XLSX, os.path.join(tempfile.gettempdir(), "ap-status-backup.xlsx"))
    today = datetime.date.today().isoformat()
    for i in range(0, len(args), 3):
        tid, status, note = args[i:i + 3]
        if tid not in rows:
            sys.exit(f"unknown task {tid}")
        if status not in ALLOWED:
            sys.exit(f"bad status {status!r}")
        r = rows[tid]
        ws.cell(r, 6).value = status
        old = ws.cell(r, 9).value
        ws.cell(r, 9).value = (str(old) + " " if old else "") + f"[{today}] {note}"
    # extend the drop-down list with the scope-cut status
    for dv in ws.data_validations.dataValidation:
        if dv.formula1 and "Aguarda aprovação" in dv.formula1 and "Sob demanda" not in dv.formula1:
            dv.formula1 = '"' + ",".join(ALLOWED) + '"'
    wb.save(XLSX)
    out = tempfile.mkdtemp()
    soffice = "/Applications/LibreOffice.app/Contents/MacOS/soffice"
    if os.path.exists(soffice):
        subprocess.run([soffice, "--headless", "--convert-to", "xlsx", "--outdir", out, XLSX],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        chk = openpyxl.load_workbook(os.path.join(out, os.path.basename(XLSX)), data_only=True)
        bad = [(s.title, c.coordinate, c.value) for s in chk for row in s.iter_rows() for c in row
               if isinstance(c.value, str) and c.value.startswith("#")]
        print("recalc errors:", bad if bad else "none")


main()
