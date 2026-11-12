#!/usr/bin/env python3
"""Isolated Windows load/API smoke test for the experimental FBNeo/libretro DLL.
Does not load any game, BIOS, CHD, profile, save state, or Steamworks.
Run only against an experimental build in GitHub Actions or an isolated folder.
"""
import ctypes
import pathlib
import sys

class RetroSystemInfo(ctypes.Structure):
    _fields_ = [
        ("library_name", ctypes.c_char_p),
        ("library_version", ctypes.c_char_p),
        ("valid_extensions", ctypes.c_char_p),
        ("need_fullpath", ctypes.c_bool),
        ("block_extract", ctypes.c_bool),
    ]

def main():
    if sys.platform != "win32":
        raise SystemExit("Windows-only smoke test")
    if len(sys.argv) != 2:
        raise SystemExit("Usage: smoke_core.py path/to/experimental/rffs_core.dll")
    dll = pathlib.Path(sys.argv[1]).resolve(strict=True)
    if dll.name.lower() not in ("rffs_core.dll", "rffs_wide_v5.dll"):
        raise SystemExit("Refusing to test unexpected DLL filename")
    print("EXPERIMENTAL CORE ONLY - NO GAME, ROM, BIOS, SAVESTATE OR STEAM")
    print(f"Loading: {dll}")
    core = ctypes.CDLL(str(dll))
    core.retro_api_version.argtypes = ()
    core.retro_api_version.restype = ctypes.c_uint
    api = core.retro_api_version()
    print(f"libretro API version: {api}")
    if api != 1:
        raise SystemExit("Unsupported libretro API version")

    info = RetroSystemInfo()
    core.retro_get_system_info.argtypes = (ctypes.POINTER(RetroSystemInfo),)
    core.retro_get_system_info.restype = None
    core.retro_get_system_info(ctypes.byref(info))
    def safe(b):
        return (b or b"").decode("utf-8", "replace")
    print(f"Core name: {safe(info.library_name)}")
    print(f"Core version: {safe(info.library_version)}")
    print(f"Supported extensions: {safe(info.valid_extensions)}")
    print(f"Need full path: {bool(info.need_fullpath)}")
    if not info.library_name:
        raise SystemExit("Core reported an empty library name")
    print("PASS: Windows DLL loaded and basic libretro calls worked (NO GAME TEST)")

if __name__ == "__main__":
    main()
