@echo off

pushd build

set SDLINCLUDE="../include/SDL2-2.0.5/include"
set SDLLIB="../include/SDL2-2.0.5/lib/x64/"
set GLAD="../include/glad/include"
set Includes="../include/"
set GLM="../include/glm-0.9.8.4/"

set IncludeDirectories=-I%SDLINCLUDE% -I%GLAD% -I%Includes% -I%GLM%
set LibDirectories=-LIBPATH:%SDLLIB%
set CompilerFlags= -DDEBUG -nologo -W4 -WX -Od -Ob0 %IncludeDirectories% -Zi -MDd /D "_WINDOWS" /D "_DEBUG"
set LinkerFlags=-nologo -DEBUG %LibDirectories% SDL2.lib SDL2main.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:CONSOLE

popd
