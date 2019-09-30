@echo off

set CommonCompilerFlags=-MD -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -FC -Z7 -EHsc -I C:\C++Projects\Projects\Client-Server_RealTime\Libraries\soil\Simple_OpenGL_Image_Library -I C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glew-2.1.0-win32\glew-2.1.0\include -I C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glfw-3.3.bin.WIN64\include -I C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glm-0.9.9.5\glm
set CommonLinkerFlags= -incremental:no -opt:ref -LIBPATH:C:\C++Projects\Projects\Client-Server_RealTime\Libraries\soil\Simple_OpenGL_Image_Library\projects\VC9\x64\Release SOIL.lib -LIBPATH:C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glfw-3.3.bin.WIN64\lib-vc2017 glfw3.lib -LIBPATH:C:\C++Projects\Projects\Client-Server_RealTime\Libraries\glew-2.1.0-win32\glew-2.1.0\lib\Release\x64 glew32.lib opengl32.lib kernel32.lib shell32.lib gdi32.lib user32.lib

IF NOT EXIST Build mkdir Build
pushd Build

cl %CommonCompilerFlags%^
 ../client.cpp^
 ../engine.cpp^
 ../shaderloader.cpp^
 ../textureloader.cpp^
 ../modelloader.cpp^
 ../fontloader.cpp^
 ../renderer.cpp^
 /link %CommonLinkerFlags%

popd