@echo off
rem This build file allows for hot .dll reloading

mkdir build
pushd build

cl "..\code\metadesk_generator.cpp" /nologo /FC /GR- /Oi /WX /D Def_Windows=1 /D Def_Linux=0  /D Def_Internal=1 /D Def_Slow=1 /W4 /wd4100 /wd4189 /wd4201 /wd4505 /wd4996 /Od /MTd /Zi /link /INCREMENTAL:no /OUT:"metadesk_generator.exe"
metadesk_generator.exe


cl "..\code\klocki.cpp" /nologo /FC /GR- /Oi /WX /D Def_Windows=1 /D Def_Linux=0  /D Def_Internal=1 /D Def_Slow=1 /W4 /wd4100 /wd4189 /wd4201 /wd4505 /wd4996 /Od /MTd /Zi /LD /link /INCREMENTAL:no /OUT:"klocki.dll"

cl "..\code\win32_klocki.cpp" /nologo /FC /GR- /Oi /WX /D Def_Windows=1 /D Def_Linux=0  /D Def_Internal=1 /D Def_Slow=1 /W4 /wd4100 /wd4189 /wd4201 /wd4505 /wd4996 /Od /MTd /Zi /link /INCREMENTAL:no /OUT:"debug_klocki.exe"

popd build