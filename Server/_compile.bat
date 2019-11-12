@echo off

set CommonCompilerFlags=-MD -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -FC -Z7 -EHsc -I C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glm-0.9.9.5\glm
set CommonLinkerFlags= -incremental:no -opt:ref Ws2_32.lib kernel32.lib shell32.lib gdi32.lib user32.lib

IF NOT EXIST Build mkdir Build
pushd Build

cl %CommonCompilerFlags%^
 ../server.cpp^
 ../engine_server.cpp^
 /link %CommonLinkerFlags%

popd