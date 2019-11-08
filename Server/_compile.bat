@echo off

set CommonCompilerFlags=-MD -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -FC -Z7 -EHsc
set CommonLinkerFlags= -incremental:no -opt:ref kernel32.lib shell32.lib gdi32.lib user32.lib Ws2_32.lib

IF NOT EXIST Build mkdir Build
pushd Build

cl %CommonCompilerFlags%^
 ../server.cpp^
 /link %CommonLinkerFlags%

popd