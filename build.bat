@echo off

pushd build

set SDLINCLUDE="../includes/SDL2-2.0.5/include"
set LIB64="../lib/x64/Debug/"
set INCLUDES="../includes/"

set IncludeDirectories=-I%SDLINCLUDE% -I%INCLUDES%
set LibDirectories=-LIBPATH:%LIB64%

set CompilerFlags= -DDEBUG -nologo -W4 -WX -Od -Ob0 %IncludeDirectories% -Zi -MTd /D "_WINDOWS" /D "_DEBUG"
set LinkerFlags=-nologo -DEBUG %LibDirectories% SDL2.lib SDL2main.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:CONSOLE

popd
