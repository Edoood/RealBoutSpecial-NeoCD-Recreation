# Real Bout Special CD: experimental wider viewport

An isolated prototype is installed as D:\neogeocd rffs1\Jogar Real Bout Widescreen.exe.
The normal Jogar Real Bout Special.exe and original CHD/BIOS are unchanged.

Implemented:
- A locally built FBNeo Neo Geo subset, upstream commit
  9d82650bb9591966987a4c36ae8a1409ce0d5d68.
- 426x224 emulated output, presented in a 16:9 window. The fixed layer and
  existing sprites stay centered in their original 320-pixel region.
- Real Bout stage-map discovery at the verified SCB3 write routine 0x50b4.
  Capture the active A4 layer object and the corresponding sprite bank.
- Additional left/right columns read from the original stage map referenced
  by object+0x4c; height at +0x4a, scroll at +0x20, wrap mask at +0x5a.
- Draw those columns at the same size and palette through FBNeo's sprite
  renderer, preserving layer order. No mirrored/generated filler scenery.
- Require title ID 0x0223 and matching opcode words before recognizing layers.
  Bound/check every map and object access, height, mask and sprite zoom.
- Include discovery arrays in save states; drawing does not write to guest
  sprite RAM or program memory. All changes are in the experimental core.

Validation:
- Headless scripted 9000-frame playback reaches a fight using this CD image.
- Frames 4000, 5000, 6000: center 320x224 pixels match stock core exactly;
  the complete 2 MiB program RAM also matches stock at each capture.
- Actual player self-test boots through 4000 frames, produces non-silent
  stereo audio, restores an 8178789-byte state, and matches 60 replayed frames.
- Screenshot shows additional original beach/stair/palm/sky stage art.
- Self-tests use rffs-selftest, separate from the user's normal save profile.

Limits:
- This is not a finished patch for every stage. Other background routines,
  zoom modes, transitions and fixed-width menus may retain empty margins or
  reveal missing tiles. Camera bounds and stage wrap need further testing.
- Arena/collisions are unchanged. Off-screen object culling is still the
  original game's behavior. Steam, rollback transport and shaders are pending.
- The UI scales a 426-pixel raster to 16:9; normal CRT pixel proportions are
  preserved to integer raster rounding (426 vs ideal 426.667 pixels).

Build:
1. Place the official FBNeo source checkout at ./FBNeo.
2. Apply FBNeo-widescreen.patch to that checkout.
3. python configure_core.py
4. cmake -S . -B core-build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release
5. cmake --build core-build --parallel 4
6. Use SDL3 3.4.18 VC headers/lib and build rffs_player.cpp with
   RFFS_WIDESCREEN defined (see build_player.ps1).
7. Install rffs_core.dll as rffs-runtime/rffs_wide_v3.dll beside the separate
   widescreen player. No game or BIOS files are in the source archive.

Research reference:
https://github.com/danilopaulinodasilva/kof98recomp/blob/master/docs/game-notes.md
That project's game-specific patch was a reference for understanding the
problem; this prototype uses Real Bout's own map and display routines.


Version 2 - moving-camera artifacts:
- Original sprites are clipped to the original 320-pixel region. Neo Geo games
  park unused actor/HUD sprites just outside that region; extending their clip
  rectangle exposed stale tile strips. Extra scenery has a separate outside-only
  clipping rule and never overwrites the original center.
- Each double-buffered layer now stores its scroll, map metadata and 10 edge
  columns at the actual game upload, with a fixed 32-tile hardware stride.
  Rendering no longer reads a newer A4 object's map/scroll for an older bank.
- Layer snapshots participate in save states and are cleared on core init.
- A 6000-frame scripted test moves left/right and attacks. All 29 captured frames
  (3000..5800 at 100-frame intervals) match the stock core's center and full 2 MiB
  program RAM. Before/after captures show removal of parked sprite artifacts.
- This does not establish that every stage/camera boundary is fixed. User
  confirmation of flicker is still needed; no full-speed visual capture yet.


Version 3 - stage boundaries:
- The original camera object is reached via the layer object's parent at +0x0a.
  Its scroll is at +0x20; the original camera maximum comes from A5+0x2020.
  These values are captured with the double-buffered stage metadata.
- Wider viewport start is clamped to [0, cameraMax-(426-320)]. World sprites
  move within that viewport with an offset from 0 to 106, instead of always 53.
  The fixed layer remains centered for HUD. Guest camera, controls, positions,
  collisions and RAM are not patched.
- Edge snapshots cover relative columns -7..0 and 20..27, so the full additional
  width can appear on the stage's interior side when at a corner.
- Verified stage-art columns in tile range 0x7000..0x7fff may render beyond the
  original view to retain partially offscreen stage decorations. This rule is
  enabled only while a recognized active stage layer is present. General
  offscreen actors/HUD sprites remain clipped to prevent parked sprite leakage.
- A two-player scripted sweep reaches camera=640 and camera=0 in Mai's stage.
  All 29 sampled program RAM comparisons against the stock core are identical.
  Render captures show contiguous original scenery at both corners.
- Saving/restoring at each corner reproduces the next 60 frames exactly.
- New entry point: Jogar Real Bout Widescreen.exe -> experimental v3 player.
  Previous players and core files are preserved; a running older process must
  be closed before starting this version. The launcher --check returns 0 when
  the v3 player and core are present, without launching a game.
- Other stages and special transitions remain experimental; these tests do not
  establish full game coverage. The menu remains its original centered 4:3.