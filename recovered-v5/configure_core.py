import re
from pathlib import Path
root=Path(__file__).resolve().parent;lr=root/'FBNeo/src/burner/libretro';base=(lr/'Makefile.common').read_text();subset=(lr/'Makefile.neogeo').read_text();variables={'MAIN_FBNEO_DIR':str(root/'FBNeo/src')}
def expand(x):
 for _ in range(20):
  new=re.sub(r'\$\((\w+)\)',lambda m:variables.get(m[1],m[0]),x)
  if new==x:return x
  x=new
 return x
for line in base.splitlines():
 m=re.match(r'(\w+)\s*:=\s*(.*)',line)
 if m and not m[2].endswith('\\'):variables[m[1]]=expand(m[2])
sources=[]
for text in (base,subset):
 for block in re.finditer(r'SOURCES_(?:C|CXX)\s*\+=\s*(.*?)(?=\n[^\t ]|\Z)',text,re.S):
  for item in re.findall(r'\$\(\w+\)/[^\s\\]+\.(?:cpp|c)',block[1]):
   item=expand(item)
   if any(v in item for v in ('vfs_implementation_uwp','libretro-common/formats/libchdr/','encoding_deflate.c')):continue
   if item not in sources:sources.append(item)
# MSVC handles compiled source correctly; all Neo Geo driver declarations are
# generated from their source instead of the upstream all-systems index.
gen=root/'core-generated';gen.mkdir(exist_ok=True)
drivertext=(root/'FBNeo/src/burn/drv/neogeo/d_neogeo.cpp').read_text()
drivertext=re.sub(r'/\*.*?\*/','',drivertext,flags=re.S)
names=re.findall(r'struct BurnDriver\s+(\w+)\s*=',drivertext)
(gen/'driverlist.h').write_text(''.join('extern struct BurnDriver '+n+';\n' for n in names)+'static struct BurnDriver* pDriver[]={'+','.join('&'+n for n in names)+'};\nstruct game_sourcefile_entry {char* game_name;char* sourcefile;};\nstatic game_sourcefile_entry sourcefile_table[]={{"neocdz","neogeo/d_neogeo.cpp"},{"",""}};\n')
includes=[gen,lr/"libretro-common/include"]
for name,value in variables.items():
 if name.endswith('_DIR') and '$' not in value and Path(value).is_dir():includes.append(Path(value))
includes +=[root/'FBNeo/src/dep/libs/lib7z',root/'FBNeo/src/dep/libs/dr_libs',root/'FBNeo/src/dep/libs/stb_vorbis',root/'FBNeo/src/dep/libs/r8brain',root/'FBNeo/src/intf/cd',root/'FBNeo/src/intf/audio',root/'FBNeo/src/intf/input',root/'FBNeo/src/burner/win32',lr/'libretro-common/include',root/'FBNeo/src/dep/libs/libchdr/include',root/'FBNeo/src/dep/libs/lzma',root/'FBNeo/src/dep/libs/zlib',lr/'deps/ugui']
cm='cmake_minimum_required(VERSION 3.25)\nproject(rffs_core LANGUAGES C CXX)\nadd_library(rffs_core SHARED\n'+''.join('"'+s.replace('\\','/')+'"\n' for s in sources)+')\n'
cm+='target_include_directories(rffs_core PRIVATE\n'+''.join('"'+str(i).replace('\\','/')+'"\n' for i in dict.fromkeys(includes))+')\n'
cm+='target_compile_definitions(rffs_core PRIVATE __LIBRETRO__ LSB_FIRST BUILD_NEOGEO SUPPORT_SPLIT_DRIVER SUBSET="neogeo" INCLUDE_7Z_SUPPORT Z7_PPMD_SUPPORT Z7_ST INCLUDE_CHD_SUPPORT USE_LIBRETRO_VFS CHDR_SYSTEM_ZLIB USE_SPEEDHACKS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)\n'
cm+='target_compile_options(rffs_core PRIVATE /O2 /w)\nset_target_properties(rffs_core PROPERTIES CXX_STANDARD 17 WINDOWS_EXPORT_ALL_SYMBOLS ON)\ntarget_link_libraries(rffs_core PRIVATE winmm user32 advapi32 comdlg32)\n'
(root/'CMakeLists.txt').write_text(cm)
print('Sources:',len(sources),'drivers:',len(names))
