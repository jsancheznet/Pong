@echo off

REM To be able to compile the project you need to download the following VC Development libraries.

REM https://www.libsdl.org/projects/SDL_image/
REM https://www.libsdl.org/projects/SDL_mixer/
REM https://www.libsdl.org/projects/SDL_ttf/

REM After downloading you need to extract the files in the external directory and set the compilation variables.
REM After that just run this bat file


pushd build

set SDLINCLUDE="..\external\SDL2-2.0.10\include"
set SDLLIB64="..\external\SDL2-2.0.10\lib\x64"

set SDLIMAGEINCLUDE="..\external\SDL2_image-2.0.5\include"
set SDLIMAGELIB64="..\external\SDL2_image-2.0.5\lib\x64"

set SDLTTFINCLUDE="..\external\SDL2_ttf-2.0.15\include"
set SDLTTFLIB64="..\external\SDL2_ttf-2.0.15\lib\x64"

set SDLMIXERINCLUDE="..\external\SDL2_mixer-2.0.4\include"
set SDLMIXERLIB64="..\external\SDL2_mixer-2.0.4\lib\x64"

set IncludeDirectories=-I%SDLINCLUDE% -I%SDLIMAGEINCLUDE% -I%SDLTTFINCLUDE% -I%SDLMIXERINCLUDE%
set LibDirectories=-LIBPATH:%SDLLIB64% -LIBPATH:%SDLIMAGELIB64% -LIBPATH:%SDLTTFLIB64% -LIBPATH:%SDLMIXERLIB64%

set CompilerFlags= -nologo -O2 %IncludeDirectories% -MD /D "_WINDOWS"
set LinkerFlags=-nologo %LibDirectories% SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:WINDOWS

popd
