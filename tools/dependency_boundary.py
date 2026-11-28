"""Check that generic C translation units do not import target-private paths.

This static gate checks repository-visible source and compile-command edges. It
does not establish target selection, resolved vendor dependencies, or BSP
qualification.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import shlex
import sys


PRIVATE_COMPONENTS = {"platform", "vendor", "bsp", "private"}
INCLUDE = re.compile(r"^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]", re.MULTILINE)


def _tokens(entry: dict) -> list[str]:
    args = entry.get("arguments")
    if isinstance(args, list) and all(isinstance(item, str) for item in args):
        return args
    command = entry.get("command")
    return shlex.split(command, posix=True) if isinstance(command, str) else []


def _under(path: Path, root: Path) -> Path | None:
    try:
        return path.resolve().relative_to(root.resolve())
    except ValueError:
        return None


def _private_path(value: str) -> bool:
    return bool(PRIVATE_COMPONENTS.intersection(Path(value.replace("\\", "/")).parts))


def dependency_errors(database: object, root: Path) -> tuple[list[str], dict]:
    errors: list[str] = []
    if not isinstance(database, list) or not database:
        return ["compile database must be a non-empty command array"], {}

    core_commands = 0
    for index, entry in enumerate(database):
        if not isinstance(entry, dict) or not isinstance(entry.get("file"), str):
            errors.append(f"compile command {index}: missing source file")
            continue
        directory = Path(entry.get("directory", root))
        source = Path(entry["file"])
        if not source.is_absolute():
            source = directory / source
        relative_source = _under(source, root)
        if relative_source is None or relative_source.parts[0] != "src" or source.suffix.lower() != ".c":
            continue
        core_commands += 1
        tokens = _tokens(entry)
        if not tokens:
            errors.append(f"{relative_source.as_posix()}: missing compile command")
            continue
        includes: list[str] = []
        i = 0
        while i < len(tokens):
            token = tokens[i]
            if token in ("-I", "-isystem", "/I") and i + 1 < len(tokens):
                includes.append(tokens[i + 1])
                i += 2
                continue
            if token.startswith("-I") and len(token) > 2:
                includes.append(token[2:])
            elif token.startswith("/I") and len(token) > 2:
                includes.append(token[2:])
            i += 1
        for include in includes:
            resolved = Path(include)
            if not resolved.is_absolute():
                resolved = directory / resolved
            if _private_path(include) or (_under(resolved, root) is not None and
                                           _private_path(_under(resolved, root).as_posix())):
                errors.append(f"{relative_source.as_posix()}: private include path {include}")

    if core_commands == 0:
        errors.append("compile database contains no generic src/*.c commands")

    source_files = sorted((root / "src").rglob("*.c"))
    source_files += sorted((root / "include" / "zarc").rglob("*.h"))
    for path in source_files:
        relative = path.relative_to(root).as_posix()
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            errors.append(f"{relative}: cannot inspect includes: {exc}")
            continue
        for included in INCLUDE.findall(text):
            if _private_path(included):
                errors.append(f"{relative}: private header include {included}")

    return errors, {"generic_c_commands": core_commands,
                    "generic_sources_scanned": len(source_files),
                    "selected_targets": "NOT_ASSERTED"}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    args = parser.parse_args(argv)
    try:
        database = json.loads(args.compile_commands.read_text(encoding="utf-8"))
        errors, summary = dependency_errors(database, args.root)
    except (OSError, ValueError, TypeError) as exc:
        errors, summary = [f"cannot read compile database: {exc}"], {}
    summary.update(result="FAIL" if errors else "BOUNDARY_CHECKS_PASS",
                   errors=errors)
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
