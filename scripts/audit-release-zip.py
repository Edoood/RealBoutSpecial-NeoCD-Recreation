#!/usr/bin/env python3
"""Read-only audit of the existing v1.0.0 draft Release ZIP.
Never rewrites the ZIP or publishes the Release. Does not extract game data.
"""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import zipfile

RELEASE_TAG = "v1.0.0"
ASSET = "Real.Bout.Special.NeoCD.Recreation.v1.0.zip"
EXPECTED_ZIP_SHA256 = "ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e"
KNOWN = {
    "RealBoutLauncher.exe": "11550860d281c877ed73e0689e87ce9c04313a497ac96de7526d4b5dbefc822e",
    "Support/Jogar Real Bout Online.exe": "671768d621fe4ea13f55a9face89da4725343ea10600df7a6625fa73ada82cf6",
    "Support/rffs-runtime/rffs_wide_v5.dll": "61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434",
    "Support/rffs-runtime/estado-inicial-protegido.rffsstate": "76247262bee61ecc3c7c9745ec65ed77ad48cbeb0e4256afeb11c5608141a3c8",
}
def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()
def sha256_entry(archive, entry):
    h = hashlib.sha256()
    with archive.open(entry) as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()
def main():
    out = Path("release-zip-audit")
    out.mkdir(exist_ok=True)
    subprocess.run(["gh", "release", "download", RELEASE_TAG, "--pattern", ASSET, "--dir", str(out), "--clobber"], check=True)
    zip_path = out / ASSET
    actual = sha256_file(zip_path)
    rows = [
        "# Read-only audit of the original draft Release ZIP",
        "",
        f"- Tag: `{RELEASE_TAG}`",
        f"- Asset: `{ASSET}`",
        f"- ZIP size: {zip_path.stat().st_size:,} bytes",
        f"- ZIP SHA-256: `{actual}`",
        f"- Matches asset digest obtained from GitHub API: **{'YES' if actual == EXPECTED_ZIP_SHA256 else 'NO'}**",
        "- No files in the Release were modified or replaced.",
        "",
        "## ZIP entries",
        "",
        "| Path in archive | Uncompressed bytes | Type |",
        "| --- | ---: | --- |",
    ]
    with zipfile.ZipFile(zip_path) as z:
        files = {x.filename.replace("\\", "/"): x for x in z.infolist() if not x.is_dir()}
        for name, info in sorted(files.items()):
            category = "binary" if Path(name).suffix.lower() in (".exe", ".dll") else "file"
            rows.append(f"| `{name.replace('|','%7C')}` | {info.file_size:,} | {category} |")
        names = list(files)
        def suffix(name):
            return [n for n in names if n == name or n.endswith("/" + name)]
        rows.extend(["", "## Known binary/state hash comparison", "", "| Manifest path | In ZIP | SHA-256 matches V6 manifest |", "| --- | --- | --- |"])
        for path, expected in KNOWN.items():
            matches = suffix(path)
            digest = sha256_entry(z, matches[0]) if len(matches) == 1 else ""
            rows.append(f"| `{path}` | {'YES' if matches else 'NO'} | {'YES' if digest == expected else 'NO' if matches else 'N/A'} |")
        rows.extend(["", "## Licensing and contents review", ""])
        licenses = [n for n in names if ("licen" in n.lower() or "copying" in n.lower() or "notice" in n.lower())]
        rows.append(f"- License/notice-named files: **{len(licenses)}**" + (": " + ", ".join(f"`{x}`" for x in licenses) if licenses else "."))
        suspect = [n for n in names if n.lower().endswith((".chd", ".cue", ".bin", ".iso")) or Path(n).name.lower() in ("neogeo.zip", "neocdz.zip")]
        rows.append(f"- CHD, loose disc image or known BIOS filenames: **{len(suspect)}**" + (": " + ", ".join(f"`{x}`" for x in suspect) if suspect else "."))
        states = [n for n in names if n.lower().endswith((".rffsstate", ".fs"))]
        rows.extend(["", "## State file headers (no state payload disclosed)", ""])
        if not states:
            rows.append("- No state files found.")
        for name in states:
            with z.open(files[name]) as f:
                header = f.read(32)
            magic = header[:8].decode("ascii", errors="replace")
            size = files[name].file_size
            rows.append(f"- `{name}`: {size:,} bytes; first 8 bytes (ASCII): `{magic}`; header begins RFFSST02: **{'YES' if header.startswith(b'RFFSST02') else 'NO'}**.")
        rows.extend([
            "",
            "## Limits",
            "",
            "- This audit checks archive structure, filenames, known hashes, and limited state headers.",
            "- It does not prove a state file is free of copyrighted game data. That requires a separate rights/content review.",
            "- A valid ZIP, available source files and passing GitHub Actions build do not by themselves establish redistributability.",
            "- The recovered V5 code has not been proven to be the exact source corresponding to the distributed V6 binaries.",
            "- Steamworks redistribution rights and the use of Spacewar AppID 480 require separate authorization/terms review.",
            "- Do not publish the draft Release solely because this audit succeeds.",
        ])
    report = "\n".join(rows) + "\n"
    (out / "AUDIT_REPORT.md").write_text(report, encoding="utf-8")
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as f:
            f.write(report)
    print(report)
    if actual != EXPECTED_ZIP_SHA256:
        raise RuntimeError("ZIP asset hash changed; do not trust the inventory")
if __name__ == "__main__":
    main()
