from pathlib import Path
import csv,re,json
root=Path('docs/architecture')
def read(p):
 with p.open(encoding='utf-8-sig') as f:return list(csv.DictReader(f))
req=read(root/'12-Requirements-Verification.csv')
alloc=read(root/'realization/17-Development-Allocation.csv')
findings=read(root/'15-Review-Findings.csv')
assert len(req)==len(alloc)==111
assert len({r['ID'] for r in req})==len({r['Verification_ID'] for r in req})==111
assert {r['ID'] for r in req}=={r['Requirement'] for r in alloc}
assert all(r['Status']=='PROPOSED' for r in req)
ids={r['ID'] for r in req}
assert all('ZARC-'+k in ids for f in findings for k in f['Requirement_suffixes'].split(';'))
assert len(findings)==28
assert {p.stem for p in (root/'diagrams').glob('*.mmd')}=={f'D{i:02}' for i in range(1,23)}
for p in [Path('CMakePresets.json'),Path('ZARC.code-workspace'),Path('.vscode/tasks.json')]:json.loads(p.read_text(encoding='utf-8-sig'))
print('PASS: 111 requirements and allocations, 28 findings, 22 diagrams, JSON configuration')
