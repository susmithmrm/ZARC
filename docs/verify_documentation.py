from pathlib import Path
import re
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
ARCH = ROOT / "docs" / "architecture"
documents = [ROOT / "README.md", *ROOT.glob("docs/**/*.md"), *ROOT.glob("platform/**/*.md")]
tables = links = 0
for document in documents:
    text = document.read_text(encoding="utf-8")
    assert "\ufffd" not in text, document
    # Fenced examples are not Markdown prose or tables.
    prose = re.sub(r"(?ms)^\s*```[^\n]*\n.*?^\s*```\s*$", "", text)
    for m in re.finditer(r"(?m)(?:^\|.*\n)+", prose):
        rows = m[0].splitlines()
        assert len(rows) >= 2 and re.fullmatch(r"\|(?:\s*:?-{3,}:?\s*\|)+", rows[1]), document
        assert len({len(re.split(r"(?<!\\)\|", row)) for row in rows}) == 1, document
        assert m.start() == 0 or prose[:m.start()].endswith("\n\n"), document
        assert m.end() == len(prose) or prose[m.end():].startswith("\n"), document
        tables += 1
    for match in re.finditer(r"\[[^\]\n]+\]\(([^)\s]+)\)", prose):
        target, _, fragment = match[1].partition("#")
        if re.match(r"https?://|mailto:", target):
            continue
        destination = document if not target else document.parent / unquote(target)
        assert destination.exists(), (document, target)
        if fragment and destination.suffix == ".md":
            body = destination.read_text(encoding="utf-8")
            anchors = set(re.findall(r'<a id="([^"]+)">', body))
            anchors.update(re.sub(r"[^\w\- ]", "", h.lower()).replace(" ", "-")
                           for h in re.findall(r"(?m)^#{1,6} (.+)$", body))
            assert fragment in anchors, (document, target, fragment)
        links += 1
atlas = (ARCH / "diagrams/README.md").read_text(encoding="utf-8")
blocks = re.findall(r"\[Editable Mermaid source\]\((D\d{2}\.mmd)\)\n\n```mermaid\n(.*?)\n```", atlas, re.S)
assert len(blocks) == 22 and len({name for name, _ in blocks}) == 22
for name, body in blocks:
    assert body == (ARCH / "diagrams" / name).read_text(encoding="utf-8").strip(), name
print(f"PASS: {len(documents)} documents, {tables} tables, {links} internal links, 22 synchronized diagram blocks")
