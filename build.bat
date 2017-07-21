@echo off

pushd build

set SDLINCLUDE="../includes/SDL2-2.0.5/include"
set SDLIMAGE="../includes/SDL2_image-2.0.1/include"
set SDLTTF="../includes/SDL2_ttf-2.0.14/include"
set LIB64="../lib/x64/Debug/"
set SDLIMAGELIB="../includes/SDL2_image-2.0.1/lib/x64"
set SDLTTFLIB="../includes/SDL2_ttf-2.0.14/lib/x64"
set SDLMIXERINCLUDE="../includes/SDL2_mixer-2.0.1/include"
set SDLMIXERLIB="../includes/SDL2_mixer-2.0.1/lib/x64"
set INCLUDES="../includes/"

set IncludeDirectories=-I%SDLINCLUDE% -I%SDLIMAGE% -I%SDLTTF% -I%SDLMIXERINCLUDE% -I%INCLUDES%
set LibDirectories=-LIBPATH:%LIB64% -LIBPATH:%SDLIMAGELIB% -LIBPATH:%SDLTTFLIB% -LIBPATH:%SDLMIXERLIB%

set CompilerFlags= -DDEBUG -nologo -W4 -WX -Od -Ob0 %IncludeDirectories% -Zi -MTd /D "_WINDOWS" /D "_DEBUG"
set LinkerFlags=-nologo -DEBUG %LibDirectories% SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:CONSOLE

popd
