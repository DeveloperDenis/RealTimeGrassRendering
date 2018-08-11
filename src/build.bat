@echo off

set exe_file_name="grass_rendering.exe"

set flags=/nologo /DDEBUG /DDENIS_WIN32 /D_CRT_SECURE_NO_WARNINGS /Gm- /GR- /EHsc /Zi /FC /W4 /WX /wd4100 /wd4101 /wd4127 /wd4505 /wd4189 /wd4201 /wd4706 /wd4702
set linker_flags=/incremental:no
set includes=/I ../src/

mkdir ..\build 2> NUL

pushd ..\build\

cl %flags% %includes% ..\src\main.cpp /Fe%exe_file_name% /link %linker_flags% user32.lib gdi32.lib opengl32.lib

popd
