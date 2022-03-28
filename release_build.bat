@echo off
rem This build file compiles the game into single .exe file (no .dll)

mkdir build
pushd build

cl "..\code\metadesk_generator.cpp" /nologo /FC /GR- /Oi /WX /D Def_Windows=1 /D Def_Internal=1 /D Def_Slow=1 /W4 /wd4100 /wd4189 /wd4201 /wd4505 /wd4996 /Od /MTd /Zi /link /INCREMENTAL:no /OUT:"metadesk_generator.exe"
metadesk_generator.exe


rem Add "/D Def_Profile=1" to add Superluminal profiler support
cl "..\code\win32_klocki.cpp" /nologo /FC /GR- /Oi /WX /D Def_Windows=1 /D Def_Internal=0 /D Def_Slow=0 /D Def_Ship=1 /W4 /wd4100 /wd4189 /wd4201 /wd4505 /wd4996 /Ox /fp:fast /MT /Zi /link /INCREMENTAL:no /OPT:REF /RELEASE /OUT:"release_klocki.exe"

popd build