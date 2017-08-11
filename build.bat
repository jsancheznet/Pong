@echo off

pushd build

set SDLINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2-devel-2.0.5-VC\SDL2-2.0.5\include"
set SDLLIB64="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2-devel-2.0.5-VC\SDL2-2.0.5\lib\x64"

set SDLIMAGEINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_image-devel-2.0.1-VC\SDL2_image-2.0.1\include"
set SDLIMAGELIB64="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_image-devel-2.0.1-VC\SDL2_image-2.0.1\lib\x64"

set SDLTTFINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_ttf-devel-2.0.14-VC\SDL2_ttf-2.0.14\include"
set SDLTTFLIB64="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_ttf-devel-2.0.14-VC\SDL2_ttf-2.0.14\lib\x64"

set SDLMIXERINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_mixer-devel-2.0.1-VC\SDL2_mixer-2.0.1\include"
set SDLMIXERLIB64="C:\Users\Jsanchez\Dropbox\Projects\Libraries\SDL2_mixer-devel-2.0.1-VC\SDL2_mixer-2.0.1\lib\x64"

set IncludeDirectories=-I%SDLINCLUDE% -I%SDLIMAGEINCLUDE% -I%SDLTTFINCLUDE% -I%SDLMIXERINCLUDE%
set LibDirectories=-LIBPATH:%SDLLIB64% -LIBPATH:%SDLIMAGELIB64% -LIBPATH:%SDLTTFLIB64% -LIBPATH:%SDLMIXERLIB64%

REM set CompilerFlags= -DDEBUG -nologo -W4 -WX -Ox -Ob0 %IncludeDirectories% -Zi -MDd /D "_WINDOWS" /D "_DEBUG"
REM set LinkerFlags=-nologo -DEBUG %LibDirectories% SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib

set CompilerFlags= -nologo -Ox %IncludeDirectories% -MT /D "_WINDOWS"
set LinkerFlags=-nologo %LibDirectories% SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:WINDOWS

popd
