#!/usr/bin/env python3
"""Make a separate V6 ZIP with license notices. Never overwrites original or Release.
Usage: python scripts/prepare-v6-license-copy.py path/to/original.zip
"""
from __future__ import annotations
import hashlib
import copy
from pathlib import Path
import sys
import zipfile

ORIGINAL_SHA = "ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e"
LICENSES = [
    "FBNeo-license.txt",
    "LICENSE-GGPO.txt",
    "LICENSE-ImGui.txt",
    "LICENSE-SDL3.txt",
]
ORIGINAL_FILES = {
    "RealBoutLauncher.exe": "11550860d281c877ed73e0689e87ce9c04313a497ac96de7526d4b5dbefc822e",
    "Support/Jogar Real Bout Online.exe": "671768d621fe4ea13f55a9face89da4725343ea10600df7a6625fa73ada82cf6",
    "Support/rffs-runtime/rffs_wide_v5.dll": "61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434",
    "Support/rffs-runtime/estado-inicial-protegido.rffsstate": "76247262bee61ecc3c7c9745ec65ed77ad48cbeb0e4256afeb11c5608141a3c8",
}
NOTES = """Real Bout Special NeoCD Recreation — third-party notices

This is an experimental distribution-preparation COPY of the original V6 ZIP.
All original entries are preserved without content changes. The original archive
and its GitHub draft Release attachment are not changed by this script.

- The included FBNeo, SDL3, GGPO and Dear ImGui license texts are copied from
  the project's THIRD_PARTY_LICENSES directory.
- Those notices do not substitute for publication of the V6-corresponding
  modified source or proof of permission to redistribute other proprietary files.
- steam_api64.dll: Steamworks proprietary component; permission/terms and the
  AppID 480 usage still need to be resolved for a public release.
- Visual C++ runtime DLLs: redistribution terms need to be verified.
- The save-state file may include game memory; copyright concerns depend on
  actual contents. No infringement or free redistributability has been established.
- Game images (CHD/ROM) and BIOS are not bundled. Players supply their own
  files they are entitled to use.
- This package is not a legally cleared public release.
"""
def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()
def main():
    if len(sys.argv) != 2:
        raise SystemExit("Usage: python scripts/prepare-v6-license-copy.py ORIGINAL_V6.zip")
    original = Path(sys.argv[1]).resolve()
    repo = Path(__file__).resolve().parents[1]
    if not original.is_file():
        raise SystemExit(f"Original ZIP missing: {original}")
    if digest(original.read_bytes()) != ORIGINAL_SHA:
        raise SystemExit("This ZIP does not match the audited original V6 SHA-256. No new ZIP was created.")
    license_sources = [(repo / "THIRD_PARTY_LICENSES" / n) for n in LICENSES]
    missing = [str(p) for p in license_sources if not p.is_file()]
    if missing:
        raise SystemExit("Missing license text(s): " + ", ".join(missing))
    out = original.with_name("Real.Bout.Special.NeoCD.Recreation.v1.0-LICENCAS-COPIA-NAO-PUBLICAR.zip")
    if out.exists():
        raise SystemExit(f"Refusing to overwrite: {out}")
    with zipfile.ZipFile(original, "r") as src:
        if src.testzip() is not None:
            raise SystemExit("Source ZIP integrity test failed")
        entry_names = set(src.namelist())
        for name, expected in ORIGINAL_FILES.items():
            if name not in entry_names or digest(src.read(name)) != expected:
                raise SystemExit(f"Original V6 file missing or hash mismatch: {name}")
        reserved = {f"THIRD_PARTY_LICENSES/{p.name}" for p in license_sources}
        reserved.add("AVISOS_DE_DISTRIBUICAO.txt")
        if entry_names.intersection(reserved):
            raise SystemExit("ZIP already contains a reserved notice/license filename; refusing to overwrite")
        with zipfile.ZipFile(out, "x", allowZip64=True) as dst:
            for entry in src.infolist():
                # Copy original entries (including ZipInfo metadata) without modifying content.
                dst.writestr(copy.copy(entry), src.read(entry.filename))
            for p in license_sources:
                dst.writestr("THIRD_PARTY_LICENSES/" + p.name, p.read_bytes(), compress_type=zipfile.ZIP_DEFLATED)
            dst.writestr("AVISOS_DE_DISTRIBUICAO.txt", NOTES.encode("utf-8"), compress_type=zipfile.ZIP_DEFLATED)
        with zipfile.ZipFile(out, "r") as dst:
            for entry in src.infolist():
                assert digest(src.read(entry)) == digest(dst.read(entry.filename)), entry.filename
            assert len(dst.infolist()) == len(src.infolist()) + len(license_sources) + 1
    print(f"New separate ZIP created: {out}")
    print(f"Original SHA-256: {digest(original.read_bytes())}")
    print(f"New SHA-256: {digest(out.read_bytes())}")
    print("Existing original game files verified unchanged.")
    print("LICENSES ADDED; legal compliance NOT YET CERTIFIED. Do not publish automatically.")
if __name__ == "__main__":
    main()
