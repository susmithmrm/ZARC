from pathlib import Path
import csv,re,json
root=Path('docs/architecture')
def read(p):
 with p.open(encoding='utf-8-sig') as f:return list(csv.DictReader(f))
req=read(root/'12-Requirements-Verification.csv')
alloc=read(root/'realization/17-Development-Allocation.csv')
findings=read(root/'15-Review-Findings.csv')
assert len(req)==len(alloc) and len(req)>0
assert len({r['ID'] for r in req})==len({r['Verification_ID'] for r in req})==len(req)
assert {r['ID'] for r in req}=={r['Requirement'] for r in alloc}
assert all(r['Status']=='PROPOSED' for r in req)
assert all('shall' in r['Requirement'] and r['Acceptance'] and r['Profiles'] for r in req)
assert len({r['Requirement'].casefold().strip() for r in req})==len(req)
by_id={r['ID']:r for r in req}
assert all((a['Design'],a['Acceptance condition'],a['Verification ID'])==(by_id[a['Requirement']]['Design_ref'],by_id[a['Requirement']]['Acceptance'],by_id[a['Requirement']]['Verification_ID']) for a in alloc)
ids={r['ID'] for r in req}
assert all('ZARC-'+k in ids for f in findings for k in f['Requirement_suffixes'].split(';'))
assert len(findings)==28
assert {p.stem for p in (root/'diagrams').glob('*.mmd')}=={f'D{i:02}' for i in range(1,23)}
for p in [Path('CMakePresets.json'),Path('ZARC.code-workspace'),Path('.vscode/tasks.json')]:json.loads(p.read_text(encoding='utf-8-sig'))
print('PASS: requirements and allocations, 28 findings, 22 diagrams, JSON configuration')
