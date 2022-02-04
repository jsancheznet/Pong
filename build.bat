@echo off

pushd build

set SDLINCLUDE="..\external\SDL2-2.0.20\include"
set SDLLIB64="..\external\SDL2-2.0.20\lib\x64"

set SDLIMAGEINCLUDE="..\external\SDL2_image-2.0.5\include"
set SDLIMAGELIB64="..\external\SDL2_image-2.0.5\lib\x64"

set SDLTTFINCLUDE="..\external\SDL2_ttf-2.0.18\include"
set SDLTTFLIB64="..\external\SDL2_ttf-2.0.18\lib\x64"

set SDLMIXERINCLUDE="..\external\SDL2_mixer-2.0.4\include"
set SDLMIXERLIB64="..\external\SDL2_mixer-2.0.4\lib\x64"

set IncludeDirectories=-I%SDLINCLUDE% -I%SDLIMAGEINCLUDE% -I%SDLTTFINCLUDE% -I%SDLMIXERINCLUDE%
set LibDirectories=-LIBPATH:%SDLLIB64% -LIBPATH:%SDLIMAGELIB64% -LIBPATH:%SDLTTFLIB64% -LIBPATH:%SDLMIXERLIB64%

set CompilerFlags= -nologo -O2 %IncludeDirectories% -MD /D "_WINDOWS"
set LinkerFlags=-nologo %LibDirectories% SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib shell32.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:WINDOWS

popd
